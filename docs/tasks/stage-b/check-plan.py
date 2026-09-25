#!/usr/bin/env python3
"""Read-only checks for the Stage B planning snapshot; no runtime acceptance."""
import collections
import hashlib
import json
from pathlib import Path
import re
import sys
from urllib.parse import unquote

PLAN = Path(__file__).resolve().parent
ROOT = PLAN.parents[2]
CANONICAL = ROOT / "docs/capabilities"
REQUIRED_SNAPSHOTS = frozenset(("manifest.json", "native-coverage.json",
                                "reconciliation.json", "native-evidence.json",
                                "inventories/index.json"))


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def read_json(path):
    return json.loads(path.read_text())


def allocation_digest(row):
    contract = {k: v for k, v in row.items()
                if k not in ("implementation", "reason", "evidenceStatus", "evidenceIds")}
    return hashlib.sha256(json.dumps(contract, ensure_ascii=True, sort_keys=True,
                                    separators=(",", ":")).encode()).hexdigest()


def anchors(path):
    result = set()
    used = collections.Counter()
    contents = path.read_text(errors="replace")
    result.update(re.findall(r'<a\s+id="([^"]+)"', contents))
    for heading in re.findall(r"^#{1,6}\s+(.+)$", contents, re.MULTILINE):
        heading = re.sub(r"\[([^]]+)\]\([^)]*\)", r"\1", heading)
        slug = re.sub(r"[^\w\- ]", "", heading.lower()).replace(" ", "-")
        result.add(slug if not used[slug] else f"{slug}-{used[slug]}")
        used[slug] += 1
    return result


def check():
    errors = []
    def require(condition, message):
        if not condition:
            errors.append(message)

    data = read_json(PLAN / "coverage.json")
    manifest = read_json(CANONICAL / "manifest.json")
    ledger = read_json(CANONICAL / "native-coverage.json")
    capabilities = {c["id"]: c for c in manifest["capabilities"]}
    coverage = {c["capabilityId"]: c for c in ledger["coverage"]}
    current_b = {cid for cid, row in coverage.items() if row["acceptanceStage"] == "B"}
    require(data["schemaVersion"] == 1 and data["stage"] == "B", "Unexpected planning schema/stage")
    snapshots = data.get("canonicalSha256")
    snapshot_shape_valid = isinstance(snapshots, dict) and set(snapshots) == REQUIRED_SNAPSHOTS
    require(snapshot_shape_valid, "canonicalSha256 must contain exactly the five required canonical paths")
    snapshots = snapshots if isinstance(snapshots, dict) else {}
    canonical_unchanged = snapshot_shape_valid
    for name in sorted(REQUIRED_SNAPSHOTS):
        expected = snapshots.get(name)
        valid_digest = isinstance(expected, str) and re.fullmatch(r"[0-9a-f]{64}", expected) is not None
        require(valid_digest, f"Missing or invalid SHA-256 for required canonical path: {name}")
        matches = valid_digest and digest(CANONICAL / name) == expected
        if valid_digest:
            require(matches, f"Canonical snapshot changed: {name}")
        canonical_unchanged = canonical_unchanged and matches

    tickets = {t["id"]: t for t in data["tickets"]}
    require(len(tickets) == len(data["tickets"]), "Duplicate ticket ID")
    require(len({t["path"] for t in tickets.values()}) == len(tickets), "Duplicate ticket path")
    rows = {c["id"]: c for c in data["capabilities"]}
    require(len(rows) == len(data["capabilities"]), "Duplicate capability mapping")
    require(set(rows) == current_b, f"Owner denominator differs: missing={sorted(current_b-set(rows))}, extra={sorted(set(rows)-current_b)}")
    require(len(current_b) == 508, "Reviewed B scope changed; re-review and update planning snapshot explicitly")
    owner_counts = collections.Counter(cid for t in tickets.values() for cid in t["owns"])
    require(set(owner_counts) == current_b and all(n == 1 for n in owner_counts.values()), "Ticket ownership is not exactly once for every B ID")
    listed_files = set()
    for tid, ticket in tickets.items():
        path = PLAN / ticket["path"]
        require(path.resolve().parent == PLAN, f"Ticket outside planning directory: {tid}")
        require(path.is_file(), f"Missing ticket file: {tid}")
        listed_files.add(path.name)
        deps = ticket["dependsOn"]
        require(len(set(deps)) == len(deps), f"Duplicate ticket dependency: {tid}")
        require(all(dep in tickets and dep != tid for dep in deps), f"Unknown/self ticket dependency: {tid}")
        if not path.is_file():
            continue
        text = path.read_text()
        region = re.search(r"<!-- owned-capabilities:start -->(.*?)<!-- owned-capabilities:end -->", text, re.S)
        require(region is not None, f"Missing ownership region: {tid}")
        if region:
            owned = re.findall(r"^\| `(capability\.[^`]+)` \|", region[1], re.MULTILINE)
            require(owned == ticket["owns"], f"Markdown/JSON owners differ: {tid}")
        for reference in re.findall(r"\bSV-B-\d{3}\b", text):
            require(reference in tickets, f"Unknown ticket reference {reference} in {tid}")
        for reference in re.findall(r"`(capability\.[^`]+)`", text):
            require(reference in capabilities, f"Unknown capability reference {reference} in {tid}")
    require({p.name for p in PLAN.glob("SV-B-*.md")} == listed_files, "Orphan/missing ticket Markdown file")

    ancestors = {}
    pending = dict(tickets)
    order = []
    while pending:
        ready = [tid for tid, ticket in pending.items()
                 if all(dep in ancestors for dep in ticket["dependsOn"])]
        if not ready:
            errors.append("Ticket dependency cycle/unresolved reference: " + ", ".join(pending))
            break
        for tid in ready:
            deps = pending[tid]["dependsOn"]
            ancestors[tid] = set(deps).union(*(ancestors[d] for d in deps))
            order.append(tid)
            del pending[tid]
    # Runtime subsets name the real early implementation producer independently
    # from the sole complete-outcome owner. Integration checks may occur later;
    # they never act as reverse implementation dependencies or accept a subset.
    subsets = data.get("productionSubsets", [])
    require(len({v["id"] for v in subsets}) == len(subsets), "Duplicate production subset ID")
    for subset in subsets:
        producer = subset["producerTicket"]
        require(producer in tickets, f"Unknown subset producer: {subset['id']}")
        require(bool(subset["scope"]) and bool(subset["checks"]), f"Unspecified production subset: {subset['id']}")
        full_owners = {rows[c]["ownerTicket"] for c in subset["capabilities"] if c in rows}
        require(all(c in rows for c in subset["capabilities"]), f"Unknown/non-B subset capability: {subset['id']}")
        require(set(subset["fullOwnerTickets"]) == full_owners, f"Wrong subset full ownership: {subset['id']}")
        for consumer in subset["consumerTickets"]:
            require(consumer in tickets and (consumer == producer or producer in ancestors.get(consumer, set())),
                    f"Runtime subset unavailable to consumer: {subset['id']} → {consumer}")
        for owner in full_owners:
            require(owner == producer or producer in ancestors.get(owner, set()),
                    f"Full owner does not reuse early producer: {subset['id']} → {owner}")
    for tid, ticket in tickets.items():
        checks = ticket.get("integrationChecks", [])
        require(len(checks) == len(set(checks)), f"Duplicate integration check: {tid}")
        for integration in checks:
            require(integration in tickets and tid in ancestors.get(integration, set()),
                    f"Integration check lacks its producer or is cyclic: {tid} → {integration}")
    foundation = set()
    for cid, row in rows.items():
        if cid not in coverage:
            continue
        allocation = coverage[cid]
        owner = row["ownerTicket"]
        require(owner in tickets and cid in tickets.get(owner, {}).get("owns", []), f"Bad primary owner for {cid}")
        require(row["prerequisites"] == allocation["prerequisites"], f"Prerequisites drift for {cid}")
        require(row["obligationIds"] == [o["id"] for o in allocation["evidenceObligations"]], f"Obligations drift for {cid}")
        require(row["sourceIds"] == capabilities[cid]["sources"], f"Sources drift for {cid}")
        require(row["allocationSha256"] == allocation_digest(allocation), f"Allocation content drift for {cid}")
        for prerequisite in allocation["prerequisites"]:
            if prerequisite in current_b:
                dep_owner = rows.get(prerequisite, {}).get("ownerTicket")
                require(dep_owner == owner or dep_owner in ancestors.get(owner, set()),
                        f"Owner readiness missing: {cid} needs {prerequisite} ({dep_owner})")
            else:
                require(coverage.get(prerequisite, {}).get("acceptanceStage") == "A", f"Non-A external prerequisite: {cid} → {prerequisite}")
                foundation.add(prerequisite)

    # Check only newly authored planning links, not historical embedded audit links.
    files = [ROOT / "docs/sv-stage-b-spec.md", PLAN / "README.md"] + [PLAN / t["path"] for t in tickets.values()]
    anchor_cache = {}
    link_count = 0
    for file in files:
        if not file.is_file():
            require(False, f"Missing planning document {file}")
            continue
        text = file.read_text()
        for reference in re.findall(r"\bSV-B-\d{3}\b", text):
            require(reference in tickets, f"Unknown ticket reference {reference} in {file.name}")
        for target in re.findall(r"\[[^\]\n]*\]\(([^)\n]+)\)", text):
            if re.match(r"[a-z]+://", target):
                continue
            link_count += 1
            target = unquote(target.strip("<>"))
            name, _, fragment = target.partition("#")
            path = (file.parent / name).resolve() if name else file
            require(path.exists(), f"Broken local link in {file.name}: {target}")
            if not path.is_file() or not fragment:
                continue
            if re.fullmatch(r"L\d+", fragment):
                require(1 <= int(fragment[1:]) <= len(path.read_text(errors="replace").splitlines()), f"Invalid line link in {file.name}: {target}")
            elif path.suffix == ".md":
                if path not in anchor_cache:
                    anchor_cache[path] = anchors(path)
                require(fragment in anchor_cache[path], f"Missing Markdown anchor in {file.name}: {target}")
    result = {"status": "valid" if not errors else "invalid", "scope": "planning-only",
              "bCapabilities": len(current_b), "mappedOwners": len(rows),
              "tickets": len(tickets), "owningTickets": sum(bool(t["owns"]) for t in tickets.values()),
              "milestonesAndGates": sum(not t["owns"] for t in tickets.values()),
              "dag": "acyclic" if not pending else "invalid", "localLinksChecked": link_count,
              "aFoundationPrerequisites": sorted(foundation), "canonicalFilesUnchanged": bool(canonical_unchanged), "productionSubsets": len(subsets),
              "integrationCheckLinks": sum(len(t.get("integrationChecks", [])) for t in tickets.values()),
              "errors": errors}
    print(json.dumps(result, ensure_ascii=False, indent=2))
    return 1 if errors else 0


if __name__ == "__main__":
    try:
        sys.exit(check())
    except (OSError, ValueError, KeyError) as error:
        print(json.dumps({"status": "unavailable", "error": str(error)}, ensure_ascii=False))
        sys.exit(2)

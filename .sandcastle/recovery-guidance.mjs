// Recovery instructions belong to the failed checkpoint, never to every agent
// subsequently launched by the workflow. In particular, independent
// verification must not inherit a proposer's corrective instructions.
export function recoveryTarget(state, incident) {
  if (incident.phase === "contract") {
    const substage = state.contractPending?.step;
    return { phase: "contract", substage, ...(substage === "parallel" && Number.isInteger(state.contractPending?.failedGroupIndex)
      ? { groupIndex: state.contractPending.failedGroupIndex } : {}) };
  }
  if (incident.phase === "plan-validate") return { phase: "plan" };
  if (incident.phase === "parallel-work") return {
    phase: "parallel-work",
    ticketPaths: state.wave?.members.filter((member) => ["failed", "blocked"].includes(member.status)).map((member) => member.ticket.path) ?? [],
  };
  return { phase: incident.phase };
}

function technicalHint(incident) {
  if (incident.category === "agent_timeout") return "The previous read-only agent timed out. Complete the same independent inspection within the runtime limit.";
  if (incident.category === "agent_failure") return "The previous read-only agent failed to run. Return a complete response matching the output schema.";
  if (incident.category === "schema_mount") return "The previous read-only agent could not access its output schema. Return a complete schema-conforming response.";
  return "The previous read-only response could not be parsed or validated. Return complete schema-conforming JSON.";
}

export function guidanceFor(state, target) {
  const saved = state.recoveryAdviceTarget;
  if (saved) {
    if (saved.phase !== target.phase || (saved.substage && saved.substage !== target.substage) ||
      (saved.groupIndex !== undefined && saved.groupIndex !== target.groupIndex) ||
      (saved.ticketPaths && !saved.ticketPaths.includes(target.ticketPath))) return "";
    return saved.technical ? `Technical retry instruction: ${saved.text}` : `Recovery guidance: ${saved.text}`;
  }
  // Read old in-flight checkpoints without broadening their routing.
  if (state.recoveryAdvicePhase !== target.phase || !state.recoveryAdvice) return "";
  if (target.phase === "contract" && target.substage !== "parallel") return "";
  if (["review", "final-audit", "parallel-work"].includes(target.phase)) return "";
  return `Recovery guidance: ${state.recoveryAdvice}`;
}

export function guidanceForAgent(state, role) {
  if (role === "ticket planning") return guidanceFor(state, { phase: "plan" });
  if (role.startsWith("implementation ")) return guidanceFor(state, { phase: "parallel-work", ticketPath: role.slice("implementation ".length) });
  if (role.startsWith("assembly wave ")) return guidanceFor(state, { phase: "assemble" });
  return "";
}

export function recoveryGuidance(state, incident, decision) {
  const target = decision.action === "repair" && ["build", "build-repair", "test-gate", "test-repair"].includes(incident.phase)
    ? { phase: "plan" } : recoveryTarget(state, incident);
  if (["review", "final-audit"].includes(incident.phase)) return { ...target, technical: true, text: technicalHint(incident) };
  return { ...target, text: incident.phase === "plan-validate" && decision.action === "repair"
    ? `${decision.summary}\nManifest validation error: ${incident.error}` : decision.summary };
}

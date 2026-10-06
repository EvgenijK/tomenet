# 06: Verify native geometry and submission timing

**What to build:** A reviewer can use the complete Stage A scenario at the required display scales and inspect measured responsiveness tied to the actual submitted frames.

**Blocked by:** 05: Preserve input and session ownership through lifecycle changes.

**Status:** implemented; unavailable environment gates remain unverified

- [ ] Exercise 1024×768 logical minimum, 1920×1080 at 100%, 3840×2160 at 200%, and fractional 125%/150% scaling; logical layout, output pixels and input hit bounds agree.
- [ ] Pending interaction survives geometry changes without automatic layout switching. Prepare implemented text/assets at final output size and follow approved raster/resource rules.
- [ ] Measure from complete decode or local input to submission of the frame containing the corresponding model revision/outcome, using actual elapsed time in the native build.
- [ ] Apply urgent ≤20 ms, interactive ≤50 ms and background ≤200 ms budgets when exercised; coalescing retains the first outstanding deadline and each violation is reported.
- [ ] Run Linux accelerated and forced software checks and MinGW smoke under Wine including software rendering; identify the backend actually used and label virtual geometry and Wine evidence accurately.
- [ ] Controlled fixture time may drive functional cases but cannot establish real submission timing. Keep visible-response targets available for manual review rather than inventing measured hard gates.
- [ ] Publish repeatable geometry/timing commands and bounded safe results; missing environments remain unverified, and no stress/soak, global memory ceiling or pixel-perfect comparison is added.


Implementation and bounded Linux evidence: [geometry/timing report](../../../docs/sv-geometry-timing.md). All 23 available regression runners passed. Virtual geometry is explicit; MinGW/Wine and physical DPI acceptance remain unverified.

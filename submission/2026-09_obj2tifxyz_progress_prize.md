# September 2026 Vesuvius Progress Prize — vc_obj2tifxyz correctness work

**Live status checked:** September 27, 2026  
**Deadline:** September 30, 2026, 11:59 PM Pacific  
**Submission posture:** truthful, bounded, and ready for final personal/contact fields — but no longer presented as two active upstream PRs.

## Important upstream status change

This package was originally built around two independent PRs:

1. **#1859 — reject silent empty tifxyz conversions**
   - Issue: https://github.com/ScrollPrize/villa/issues/1320
   - PR: https://github.com/ScrollPrize/villa/pull/1859
   - Current status: open, not merged. It is now behind current upstream main and GitHub currently reports it as non-mergeable.
   - Its fix-specific validation remains useful evidence, but its zero-valid behavior is now also covered by the broader upstream PR #1781.

2. **#1861 — write tifxyz scale from measured final-grid density**
   - Issue: https://github.com/ScrollPrize/villa/issues/1319
   - PR: https://github.com/ScrollPrize/villa/pull/1861
   - Current status: closed, not merged.
   - On September 25, maintainer @pmh47 closed it in favor of #1781, saying #1781 subsumes it.

Broader upstream PR now covering both issue areas:

- https://github.com/ScrollPrize/villa/pull/1781
- #1781 handles measured scale and empty/degenerate-grid failure in one broader implementation, includes real-data evidence, and is currently the likely upstream path.

**Claim boundary:** I do not claim that #1859 or #1861 is the selected upstream implementation, and I do not claim priority over #1781. This submission documents the independent engineering work, current-main reproductions, regressions, cross-platform validation, and combined compatibility testing I completed before the upstream work was consolidated.

Related scale-contract evidence:
- https://github.com/ScrollPrize/villa/issues/1379

---

## Form answers

### 1. Email

Use the email address where the Vesuvius Challenge team should contact you.

### 2. Your full name

Use your real legal/contact name.

### 3. Team description

```
Individual submission — no team.
```

### Discord display name (optional)

Use your actual Vesuvius Challenge Discord display name, or leave blank.

### 4. URL(s) of your open-source / publicly available contribution

```
Zero-valid conversion PR:
https://github.com/ScrollPrize/villa/pull/1859

Scale-correctness PR (closed in favor of broader upstream #1781):
https://github.com/ScrollPrize/villa/pull/1861

Combined compatibility / validation branch:
https://github.com/Kaluzy/villa/pull/8

Underlying bug reports:
https://github.com/ScrollPrize/villa/issues/1320
https://github.com/ScrollPrize/villa/issues/1319

Broader upstream implementation that now subsumes the same issue areas:
https://github.com/ScrollPrize/villa/pull/1781

Submission evidence:
https://github.com/Kaluzy/villa/blob/main/submission/2026-09_obj2tifxyz_progress_prize.md
```

### 5. What is your contribution?

```
During September I independently investigated and implemented correctness
hardening for vc_obj2tifxyz, a Volume Cartographer / VC3D command-line tool
that converts OBJ papyrus surfaces into tifxyz surfaces used by downstream
virtual-unwrapping workflows.

The work focused on two failure modes at the same conversion boundary:

1) an OBJ conversion could rasterize zero usable geometry but still write
   x/y/z TIFFs, print a success message, and exit 0; and

2) a standalone conversion could write scale metadata in the wrong direction,
   causing downstream tools to interpret the resulting surface at the wrong
   physical/grid density.

I developed the two fixes independently, added real-CLI regressions, integrated
the tests into CI, validated them across platforms, and then tested both
changes together against the same compiled vc_obj2tifxyz executable.

UPSTREAM STATUS / LIMITATION

The upstream situation changed after this work was completed.

PR #1861 was closed on September 25 by maintainer @pmh47 in favor of PR #1781,
which subsumes the scale fix. PR #1781 also covers the empty/degenerate-grid
failure addressed by #1859 and is now the broader likely upstream solution.

I therefore do not claim that my two PRs are the selected upstream
implementation or that I have priority over #1781. I am submitting the
engineering work and evidence I completed: independent current-main
reproduction, implementation, regression tests, CI integration,
cross-platform validation, documentation analysis/correction, and a combined
compatibility test demonstrating that the two fixes work together.

A) ZERO-VALID / SILENT-SUCCESS FAILURE — #1320 / PR #1859

With normalized [0,1] UVs, vc_obj2tifxyz could rasterize zero valid points.
Previously it could still write x.tif, y.tif and z.tif containing invalid
sentinel geometry, print "Successfully converted to tifxyz format", and exit 0.
That meant automation could not distinguish an unusable empty surface from a
valid conversion.

My #1859 implementation fails closed when valid_count == 0:

- non-zero exit
- no fake x/y/z tifxyz output
- no success message
- actionable failure text
- an explicit adequate sampling density still converts the same synthetic mesh

I added a regression that invokes the actual built vc_obj2tifxyz executable,
rather than only unit-testing helper logic.

Controlled BEFORE:
- zero valid points
- exit 0
- x/y/z TIFFs written
- success message printed

AFTER:
- exit non-zero
- no x/y/z output
- no success message
- same mesh succeeds with an explicit adequate sampling density

The final #1859 implementation was validated in the fork on Linux, macOS,
Windows MSYS2/UCRT64, core CI, the real CLI regression, and CodeQL.

Issue #1320 reported that 283 published paths/OBJ segments used normalized
[0,1] UVs at the time of that report. That count belongs to the issue reporter;
I do not claim it as my own measurement.

B) TIFXYZ SCALE-CONTRACT FAILURE — #1319 / PR #1861

The tifxyz scale contract is a density: grid cells per surface/volume unit.
The reference QuadSurface implementation maps surface -> grid by multiplying by
scale and grid -> surface by dividing by scale.

Standalone vc_obj2tifxyz instead derived meta.json.scale from UV spacing /
stretch_factor. That could make a successful conversion describe a different
physical/grid extent from the 3D grid it actually produced.

Issue #1319 documented the real downstream impact on a published Scroll 1
segment. Those published-segment measurements belong to the issue reporter,
@Aleredfer, and I credit them rather than claiming that experiment as my own.

I independently reproduced the root contract error on then-current upstream
main with a deterministic 100 x 100 planar OBJ. With 20 grid intervals across
100 units, the correct density is 0.2 cells/unit.

Before:
  META_SCALE = [0.050000000745..., 0.050000000745...]
  EXPECTED = 0.2
  RESULT = MISMATCH

My patched branch:
  standalone scale = (0.199999988..., 0.199999988...)
  source-aware scale = (0.199999988..., 0.199999988...)
  obj2tifxyz scale regression: PASS

The implementation measured adjacent 3D spacing on the final rasterized grid
and stored its reciprocal density. I also added a real-CLI regression and
corrected the tifxyz scale documentation in my branch so it matched the core
QuadSurface convention.

PR #1861 is now closed because the maintainers chose the broader #1781 path.
I do not claim #1861 as an active or adopted upstream fix.

COMBINED COMPATIBILITY VALIDATION

Because the two independent fixes both changed vc_obj2tifxyz, I created a
temporary combined branch and PR to test them together rather than assuming
they were compatible:

https://github.com/Kaluzy/villa/pull/8

Against one compiled vc_obj2tifxyz executable, the combined branch passed:

- Linux CLI compile
- measured-scale regression
- zero-valid regression suite
- Linux full CI
- synthetic rendering regression
- macOS build
- Windows MSYS2/UCRT64 build
- Python test matrix
- CodeQL

Measured-scale evidence from that combined run:
  standalone scale: 0.199999988...
  source-aware scale: 0.199999988...
  scale regression: PASS

Zero-valid evidence:
  2 tests
  OK

This contribution is intentionally about conversion correctness, reproducible
failure detection, and pipeline safety. I do not claim an ink-model AUC/F1
improvement, and I do not claim that my implementation supersedes #1781.
```

---

## Evidence and attribution boundaries

- #1320's published normalized-UV segment count belongs to the issue report.
- #1319's published Scroll 1 downstream measurements belong to @Aleredfer.
- My evidence consists of the independent controlled reproductions, code,
  real-CLI regressions, CI integration, cross-platform validation, documentation
  analysis/correction, and combined compatibility validation.
- #1861 is closed and must not be described as open, merge-ready, or awaiting
  maintainer adoption.
- #1859 remains open, but #1781 now covers the same zero/degenerate-grid problem.
- I do not claim that #1861 repairs the already-published PHercParis4 metadata
  described in #1379.
- I do not claim an F1/AUC or ink-reading accuracy improvement.
- I do not claim a guaranteed prize amount.

## Why the work was originally grouped

The two bugs are at the same OBJ -> tifxyz correctness boundary:

1. geometry validity: a conversion with no usable geometry could report success;
2. geometry scale: a conversion with geometry could attach metadata that made
   downstream tools interpret the geometry at the wrong density.

The combined validation demonstrated that both corrections could coexist in the
same converter. Upstream has since consolidated these concerns more broadly in
#1781.

## Current engineering decision

Do not add more production code to #1859 or revive #1861 merely to increase the
size of this submission.

Only touch #1859 again if a maintainer asks for a rebase, conflict resolution,
or a specific review change. If that happens, preserve the regression and rerun
the real CLI / combined checks.

A new Progress Prize contribution should only be pursued if it independently
passes all four gates:

- broken on current upstream main
- real workflow impact
- genuinely unowned / not already solved
- measurable before/after with bounded implementation risk

# September 2026 Vesuvius Progress Prize — vc_obj2tifxyz conversion correctness hardening

**Status:** Ready to submit once desired; both upstream PRs are open and ready for review  
**Deadline:** September 30, 2026, 11:59 PM Pacific

## Primary contributions

1. **Reject silent empty tifxyz conversions**
   - Issue: https://github.com/ScrollPrize/villa/issues/1320
   - PR: https://github.com/ScrollPrize/villa/pull/1859

2. **Write tifxyz scale from measured final-grid density**
   - Issue: https://github.com/ScrollPrize/villa/issues/1319
   - PR: https://github.com/ScrollPrize/villa/pull/1861

Related scale-contract evidence:
- https://github.com/ScrollPrize/villa/issues/1379

---

## Form answers

### 1. Email

Use the email address where the Vesuvius Challenge team should contact you.

### 2. Your full name

Use your real full name.

### 3. Team description

```
Individual submission — no team.
```

### Discord display name (optional)

Use your actual Vesuvius Challenge Discord display name, or leave blank.

### 4. URL(s) of your open-source / publicly available contribution

```
Primary PR — prevent silent empty vc_obj2tifxyz output:
https://github.com/ScrollPrize/villa/pull/1859

Primary PR — correct vc_obj2tifxyz tifxyz scale metadata:
https://github.com/ScrollPrize/villa/pull/1861

Bug reports / real-workflow context:
https://github.com/ScrollPrize/villa/issues/1320
https://github.com/ScrollPrize/villa/issues/1319

Submission evidence:
https://github.com/Kaluzy/villa/blob/main/submission/2026-09_obj2tifxyz_progress_prize.md
```

### 5. What is your contribution?

```
I hardened vc_obj2tifxyz, a Volume Cartographer/VC3D command-line tool used to
convert OBJ papyrus surfaces into tifxyz surfaces for downstream virtual-
unwrapping workflows.

The work addresses two separate correctness failures in the same conversion
boundary.

1) Silent empty conversion — ScrollPrize/villa #1320 / PR #1859

With normalized [0,1] UVs, a default conversion can rasterize zero valid points.
Previously vc_obj2tifxyz still wrote x.tif, y.tif and z.tif containing invalid
sentinel values, printed "Successfully converted to tifxyz format", and exited
0. Automated pipelines therefore could not distinguish an unusable empty
surface from a valid conversion.

The fix makes zero-valid rasterization fail closed: it returns non-zero before
saving output and prints actionable guidance to provide an explicit sampling
density or use --tifxyz-source.

I added an end-to-end regression using the actual built vc_obj2tifxyz binary.

Controlled BEFORE:
- zero valid points
- process exits 0
- x/y/z TIFFs are written
- success message is printed
- regression fails

AFTER:
- process exits non-zero
- no fake x/y/z output is written
- success message is absent
- the same mesh still converts normally with an explicit adequate sampling
  density
- regression passes

The final #1859 head passes Linux, macOS, Windows MSYS2/UCRT64, full core CI,
the normal CLI regression, and CodeQL.

Issue #1320 reported that 283 published paths/ OBJ segments used normalized
[0,1] UVs at the time of the report.

2) Wrong tifxyz scale metadata — ScrollPrize/villa #1319 / PR #1861

Tifxyz scale is a density: grid cells per surface/volume unit. The reference
QuadSurface implementation maps surface->grid by multiplying by scale and
grid->surface by dividing by scale.

Standalone vc_obj2tifxyz instead derived meta.json.scale from UV spacing /
stretch_factor. That can make a successful conversion describe a completely
different physical extent from the 3D grid it actually produced.

#1319 documented this on published Scroll 1 segment paths/20231007101619:
the converted tifxyz stored scale 0.0005 while the measured grid spacing implied
about 0.0865 — roughly a 173x error. The downstream vc_flatten result collapsed
from a 2001x2001 conversion with about 3.53 million valid points to a 6x4
surface with only 18 valid points. Those real-data measurements belong to the
issue reporter, @Aleredfer; I credit that report rather than claiming the
experiment as my own.

I independently reproduced the same root contract error on current main with a
deterministic 100x100 planar OBJ. With 20 grid intervals across 100 units, the
correct tifxyz density is 0.2 cells/unit.

Current main:
  META_SCALE = [0.050000000745..., 0.050000000745...]
  EXPECTED = 0.2
  RESULT = MISMATCH

Patched:
  standalone scale = (0.199999988..., 0.199999988...)
  source-aware scale = (0.199999988..., 0.199999988...)
  obj2tifxyz scale regression: PASS

The fix measures spacing on the final rasterized 3D grid and stores its
reciprocal density. --tifxyz-source remains unchanged and preserves an explicit
source scale verbatim.

I also corrected lasagna/tifxyz_format.md so the documented scale convention
matches the reference QuadSurface implementation. This matters beyond #1319:
#1379 documents the same convention error in the opposite direction, where a
published PHercParis4 outer_shell stores ~20 instead of ~0.05 and vc_flatten
attempts an approximately 1.3 TB allocation. PR #1861 does not claim to repair
that already-published file, but it removes the contradictory format guidance
that could create new instances.

The #1861 regression is wired into the repository's normal Linux CLI CI job, so
the scale contract is tested using the real built executable on future PRs.

Validation on the clean branch rebased to current upstream main includes:
- focused real-CLI scale regression
- normal Linux CLI CI with the new regression
- Linux and macOS package builds
- Windows MSYS2/UCRT64 native build
- Linux base tests
- VC3D compile/smoke
- Lasagna/fiber/GUI tests
- Flatboi/PaStiX compile
- Python zarr 2.18.7 and 3.2.1 matrix
- CodeQL

Together these changes make OBJ->tifxyz conversion safer in two ways:
the converter can no longer silently claim success when it produced no usable
surface, and successful standalone conversions now describe the physical/grid
density of the surface they actually produced.

This is intentionally a correctness contribution rather than a claim of higher
ink-model accuracy. The goal is to prevent invalid or mis-scaled surfaces from
silently entering later virtual-unwrapping stages.
```

## Attribution / claim boundaries

- #1320's published-segment count comes from the issue report.
- #1319's published Scroll 1 measurements (173x scale error and 3.53M -> 18
  downstream collapse) come from @Aleredfer's report.
- My independently generated evidence is the current-main controlled
  reproduction, the patches, regression tests, CI integration, cross-platform
  validation, and documentation correction.
- I do not claim that PR #1861 repairs the already-published bad PHercParis4
  metadata in #1379.
- I do not claim an F1/AUC or ink-reading accuracy improvement.

## Why these belong together

Both bugs sit at the same OBJ->tifxyz boundary:

1. #1320: the tool can produce **no usable geometry** but report success.
2. #1319: the tool can produce usable geometry but attach **incorrect scale
   metadata**, causing downstream tools to interpret it incorrectly.

The combined contribution therefore hardens both the geometry-validity contract
and the geometry-scale contract of the conversion step.

# September 2026 Vesuvius Progress Prize — vc_obj2tifxyz zero-valid safety fix

**Status:** Ready to submit  
**Deadline:** September 30, 2026, 11:59 PM Pacific  
**Upstream issue:** https://github.com/ScrollPrize/villa/issues/1320  
**Upstream PR:** https://github.com/ScrollPrize/villa/pull/1859

---

## 1. Email

Use the email address where the Vesuvius Challenge team should contact you.

## 2. Your full name

Use your real full name.

## 3. Team description

```
Individual submission — no team.
```

## Discord display name (optional)

Use your actual display name in the Vesuvius Challenge Discord, or leave blank.

## 4. URL of your open source / publicly available contribution

```
Primary contribution (upstream PR):
https://github.com/ScrollPrize/villa/pull/1859

Original bug report:
https://github.com/ScrollPrize/villa/issues/1320

Contributor branch:
https://github.com/Kaluzy/villa/tree/fix/obj2tifxyz-zero-valid

Submission evidence package:
https://github.com/Kaluzy/villa/blob/main/submission/2026-09_obj2tifxyz_progress_prize.md
```

## 5. What is your contribution?

```
I fixed a silent-success failure in vc_obj2tifxyz, a Volume Cartographer/VC3D
command-line tool that converts triangular OBJ papyrus surfaces into tifxyz
surfaces for downstream virtual-unwrapping workflows.

The bug is tracked as ScrollPrize/villa #1320. With normalized [0,1] UV
coordinates, the default conversion can produce a 2x2 raster with zero valid
papyrus points. Before this patch, vc_obj2tifxyz still wrote x.tif, y.tif and
z.tif containing only invalid sentinel values, printed "Successfully converted
to tifxyz format", and returned exit code 0. Automation therefore could not
distinguish an unusable empty surface from a valid conversion.

Issue #1320 reports that the published paths/ OBJ segments use normalized [0,1]
UVs, covering 283 published segments at the time of the report.

The patch makes zero-valid rasterization fail closed. If valid_count == 0,
vc_obj2tifxyz now returns a non-zero status before saving output and prints an
actionable message explaining that normalized UVs generally need an explicit
larger stretch_factor or --tifxyz-source. It deliberately does not guess a
sampling density, so successful existing conversions keep their current
geometry and behavior.

I added an end-to-end regression that invokes the actual built
vc_obj2tifxyz executable rather than mocking the conversion.

Controlled BEFORE/AFTER evidence using the same test and build environment:

BEFORE — untouched production behavior:
- the zero-valid conversion returns success
- x.tif, y.tif and z.tif are written
- the success message remains present
- the regression fails

The recorded checks include:
  CHECK(defaultRc != 0)                         FAILED
  CHECK_FALSE(x.tif exists)                     FAILED
  CHECK_FALSE(y.tif exists)                     FAILED
  CHECK_FALSE(z.tif exists)                     FAILED
  CHECK(success message is absent)              FAILED
  0% tests passed, 1 test failed

AFTER — patched branch:
  Start 36: test_obj2tifxyz_zero_valid
  1/1 Test #36: test_obj2tifxyz_zero_valid .... Passed
  100% tests passed, 0 tests failed out of 1

The regression also verifies that the same normalized-UV synthetic mesh still
converts successfully when an adequate explicit stretch factor is supplied, so
the safety check rejects an empty conversion rather than rejecting the mesh
format itself.

Cross-platform validation on the contributor fork passed:
- full Volume Cartographer core CI
- Linux CLI compile coverage
- Linux base and specialized tests
- VC3D compile/smoke
- synthetic rendering regression
- macOS compilation
- Windows MSYS2/UCRT64 native compilation
- CodeQL
- focused end-to-end vc_obj2tifxyz regression

The production change is intentionally small and conservative: it stops one
dangerous state at the point where the program already knows that no usable
surface was produced. This prevents invalid geometry from silently entering
downstream virtual-unwrapping automation while preserving successful
conversions and standard tifxyz output behavior.

The help text was also corrected so stretch_factor is described as a sampling
density control and the normalized-UV case is called out.

Limitations / scope:
I personally reproduced the zero-valid behavior with the real vc_obj2tifxyz
binary in a controlled normalized-UV regression and verified the patched
behavior across the project's CI environments. I did not claim an ink-detection
accuracy improvement or an unwrapping-quality gain. The historical published
OBJ used in the related Scroll 1 reports was not available at the direct asset
path I tested, so I am not claiming a personal rerun on that specific historical
OBJ. The published-data blast radius comes from issue #1320's report.
```

## Terms and conditions

Review the current form terms and, if you agree, select **Yes, I agree**.

---

## Evidence summary

### Root cause / behavior
Current main detects `valid_count == 0` but previously only warned and continued to save output and report success.

### Production change
17 added production lines in `volume-cartographer/apps/src/vc_obj2tifxyz.cpp`, plus focused regression coverage and CMake registration.

### Controlled regression
- Unpatched main + test: FAILS for the exact expected reasons.
- Patched branch + same test: PASSES 1/1.

### Integration
The contributor fork validated the patch through the project's Linux, macOS and Windows build paths plus CodeQL and full core CI.

### Prize-fit
This contribution addresses an outstanding bug in a Vesuvius tool, keeps community-standard tifxyz output unchanged for successful conversions, improves automation safety, is publicly available before the monthly deadline, and includes reproducible before/after evidence.

## What this submission does NOT claim

- No claim that this improves model F1/AUC.
- No claim that it directly reveals new letters.
- No claim that every empty tifxyz in the public corpus was caused by this bug.
- No claim of a real-data run on the historical OBJ whose direct asset URL is no longer available at the tested path.

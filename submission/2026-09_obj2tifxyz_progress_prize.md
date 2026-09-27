# September 2026 technical submission note — vc_obj2tifxyz zero-valid handling

## Contribution

This contribution hardens `vc_obj2tifxyz` against a silent failure reported in ScrollPrize/villa issue #1320.

Upstream pull request:
https://github.com/ScrollPrize/villa/pull/1859

Public branch:
https://github.com/Kaluzy/villa/tree/fix/obj2tifxyz-zero-valid

Issue:
https://github.com/ScrollPrize/villa/issues/1320

## Problem

With normalized `[0,1]` UV coordinates and the default sampling density, `vc_obj2tifxyz` can rasterize zero valid grid points but still:

- write sentinel-only x/y/z TIFF files,
- print `Successfully converted to tifxyz format`, and
- exit with status 0.

A script or downstream pipeline therefore cannot distinguish an unusable empty surface from a valid conversion.

Issue #1320 reports that this failure mode applies to the published `paths/` OBJ collection, where 283 published segment OBJs used normalized `[0,1]` UVs at the time of the report.

## Implementation

PR #1859 makes `valid_count == 0` a hard failure before output is saved.

The converter now:

- returns a non-zero exit code,
- does not write fake x/y/z tifxyz output,
- does not print a success message,
- explains that normalized UVs require an explicit adequate `stretch_factor` or `--tifxyz-source`, and
- leaves successful conversions unchanged.

The implementation deliberately does not guess a replacement sampling density.

## Regression coverage

The branch adds:

`volume-cartographer/core/test/test_obj2tifxyz_zero_valid.py`

The regression invokes the real built `vc_obj2tifxyz` executable.

It checks two cases:

1. default normalized-UV conversion:
   - zero valid points,
   - non-zero exit,
   - no x/y/z TIFF output,
   - no success message;

2. the same mesh with an explicit adequate sampling density:
   - exit 0,
   - x/y/z TIFF output exists,
   - success message is present.

The regression is wired into the repository's normal Linux CLI CI workflow.

## Before / after evidence

Before the production fix, the final regression observed:

```
Valid grid points: 0 / 4 (0%)
Successfully converted to tifxyz format
```

and the process returned success.

With the fix, the same regression completes with:

```
Ran 2 tests
OK
```

## Validation

The production change was validated in the fork across:

- Linux,
- macOS,
- Windows MSYS2/UCRT64,
- core CI,
- the real CLI regression, and
- CodeQL.

Four CodeQL review threads visible on the upstream PR refer to an earlier C++ test implementation that used `std::system()`. That test was removed and replaced by the Python subprocess regression. Those four threads are resolved and outdated.

## Vesuvius data context

This is a converter correctness fix rather than an ink-model or segmentation-quality experiment.

The real-data motivation comes from issue #1320 and the published `paths/` OBJ collection. My final deterministic regression does not require downloading a scroll segment; it reproduces the same normalized-UV failure class with a synthetic OBJ so the behavior can run quickly and reliably in CI.

I do not claim a scroll-reading accuracy improvement or a new segmentation result.

## Upstream overlap

PR #1781 was opened earlier and now covers the same empty/degenerate-grid failure as part of a broader `vc_obj2tifxyz` change.

This submission does not claim priority over #1781.

The distinct engineering contribution in #1859 is the narrow fail-closed implementation and a real-binary zero-valid regression wired into the normal Linux CLI CI path. PR #1781's broader end-to-end test is currently registered behind the opt-in `VC_RUN_E2E` path.

## License

The branch is in the public `Kaluzy/villa` fork of ScrollPrize/villa and retains the repository's MIT license.

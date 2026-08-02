# Recorded working checkpoint: pre-cursive polish

This directory records the QuickShape build that restores the smoothing
behavior described during interactive testing on 2026-08-02 as
"significantly better," before cursive-loop preservation was introduced.

The recovered behavior uses:

- arc-length resampling, capped at 256 samples;
- a quadratic Savitzky-Golay/local-polynomial fit;
- a radius of `clamp(sample_count / 10, 4, 24)`;
- fixed 75% correction strength;
- no later intersection, loop-area, or topology fallback guards.

The versioned `.so` and `.tar.gz` are the rollback artifacts. They target the
official Krita 5.3.3 Qt5 x86-64 AppImage. The included launcher uses an
isolated profile and does not install into the user's normal Krita setup.

Known limitation: loops and cursive writing may be flattened or distorted.

Archive SHA-256:

`3d6b86af4e93875663e746a74b9a1066882a69ed20f62e5897bf1feefa1d182e`

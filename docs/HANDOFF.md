# Handoff to the ECE 592 group repository

This is the contribution for **Flush+Flush Task 2.1/2.2**, not Flush+Reload, Prime+Probe, overall three-PoC comparison, or Task 2.3 defense predictions. Integrate with the team rather than presenting it as a complete 20-point submission.

## Preserved versus removed

- **Preserved:** `sc/` current source, `original/` historical source, all `tools/`, all canonical eight host `results/<host>/` raw CSVs, plots, verification/provenance material, and a different historical Skylark run now in `results/skylark-repeat-20261015T172819Z-3011325/`.
- **Cleaned:** redundant nested `flush-flush-evidence/` package, top-level `flush-flush-evidence.tar.gz`, redundant `results/checklist-skylark-20261015/` identical primary data already represented by `results/skylark/`, overlapping top-level narrative docs, obsolete backup `sc/ff_bench.c.before-update` (previous source preserved in `original/`) and two rebuildable `sc/` executables. All omitted entries remain in the original `main` Git history.
- **Added:** brief entry-point README, consolidated machine-level CSV and comparison document, experiment guide, handoff notes, and explicit AI disclosure.

The original compiled binary *snapshots inside each evidence run* remain because their hashes and verifier depend on them. The cleanup does not alter any experiment code or recalculate raw data. Check an existing run with `python3 tools/verify_evidence.py results/skylark`.

## Group review before submission

1. Confirm **Charlotte replacing Thunderbird** with the course staff: the supplied PDF lists Thunderbird.
2. Confirm that the newer AI permission the student mentioned covers this documentation cleanup; the supplied written PDF prohibits AI analysis/coding.
3. Preserve complete lab provenance, raw measurements and individual commit/contribution history; do not silently remove failed/weak CPU results.
4. Confirm links work when copying to the group repo; add team-owned Flush+Reload, Prime+Probe, defense scaling analysis and the joint report separately.

No new experiment was run and no access to lab machines was attempted in this cleanup.

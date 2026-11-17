# Documentation standards

American English (en-US). Markdown only. Do not invent device results, protocol fields, or versions. When a source is silent, say so. Do not copy secrets, serial numbers, or private keys into these files. Public measurement IDs that already ship in client-side website code may be named.

## One documentation tree

All project documents live in `documentation/`: one page per topic, plus `adr/` and the Monocypher license note.

Update the page that owns the behavior. Link with a **Related Documentation** section and stable relative paths. Do not add a second root such as `docs/` for the same material.

This repository is not an npm workspace monorepo. Do not add `monorepo.md` unless that layout changes.

## Filenames

- Directories and handbook pages use lowercase kebab-case: `overview.md`, `tech-stack.md`.
- ADR files live in `documentation/adr/` and match `adr-0000-short-title.md`, with a four-digit id.
- `README.md` at the repository root, under `website/`, and under `models/` stays uppercase because those are existing entry points.

`.github/workflows/documentation.yml` checks out the tree and fails when a file under `documentation/` breaks these rules: a non-markdown file, a directory that is not kebab-case, a handbook page that is not kebab-case, or an ADR whose name does not match the pattern above.

## ADR rules

An ADR states status, context, decision, and consequences. The shared-core decision is [adr-0001](adr/adr-0001-shared-core-and-board-profiles.md). When the decision changes, add a new ADR. Do not rewrite history out of the old one.

## Changelog

User-visible changes go to the root [CHANGELOG.md](../CHANGELOG.md). [changelog-summary.md](changelog-summary.md) stays a short map of that file. Do not invent releases. The firmware label changes only in `include/Version.h`.

## Editor context

`.cursor/context.json` includes `documentation/**`. `.vscode/settings.json` does not exclude `documentation/**` from files or search. `.gitignore` ignores the rest of `.vscode/` and keeps `settings.json`.

## Related Documentation

- [Overview](overview.md)
- [Changelog summary](changelog-summary.md)
- [Commands](commands.md)

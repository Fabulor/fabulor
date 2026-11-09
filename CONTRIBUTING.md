# Contributing to Fabulor

Contributions are welcome, including bug reports, documentation, translations,
accessibility improvements, tests, and code changes.

## Before you start

Search existing issues and pull requests before starting work. For a large
feature, dependency change, or architectural change, open an issue first to
discuss the user problem, proposed approach, and compatibility concerns.

For help using Fabulor, see the [user manual](docs/user/README.md) and the
support channel listed in the [README](README.md).

Report suspected security vulnerabilities privately using the
[security policy](SECURITY.md). Do not include vulnerability details in a
public issue or pull request.

## Reporting bugs and requesting features

Use the repository's issue templates when available.

For a bug report, include:

- Your Fabulor version, Windows version and build, and installation type.
- The shortest steps that reproduce the problem.
- Expected and actual behaviour, and how often the problem occurs.
- Relevant add-ons and connection details, such as TLS, a proxy, or ZNC.
- Sanitised logs, screenshots, or crash details where useful.

Remove passwords, tokens, private messages, and other sensitive information
from reports and attachments.

For a feature request, explain the user problem, give a concrete use case,
and describe the proposed behaviour and any alternatives considered.

## Development setup

Fabulor targets Windows 11 x64 and uses a GTK4 frontend. Windows 10 use is
best-effort and unsupported. Use the supported Windows build when validating
application changes.

Start with these sources:

- [Technical documentation](docs/README.md) for architecture and component guides.
- [GTK4 build and packaging tools](tools/gtk4/README.md) for dependency validation,
  build commands, and runtime staging.
- [Windows support dependencies](tools/windows-deps/README.md) for the pinned
  OpenSSL dependency setup.
- [Build validation workflow](.github/workflows/tests.yml) for the native and
  managed build prerequisites and commands used by CI.
- [Windows build workflow](.github/workflows/windows-build.yml) for the full
  application and installer build.

Run native builds from an x64 Visual Studio developer shell. The supported
solution entry point is `win32\fabulor.sln`. Follow the workflow's prerequisite
setup and supply the validated GTK4 and support dependency roots before
building; a fresh checkout alone does not contain those dependencies.

Use the pinned dependency contracts under `tools/gtk4` and the versions in
the workflows. Avoid substituting a machine-wide GTK installation. The
repository does not currently provide a top-level Meson build; the Meson
probe in `tools/gtk4` is an isolated dependency check.

For add-on work, see the [plugin authoring guides](docs/plugin-authoring-guides.md)
and [sample plugins](samples/plugins/README.md).

## Making changes

Create a branch from the current `main` branch in your fork or authorised
checkout. Keep each change focused on one problem.

- Follow the conventions of the surrounding code and keep diffs small.
- Avoid unrelated formatting, refactoring, or dependency updates.
- Use Australian English in new documentation and user-facing prose, while
  preserving API names and established identifiers.
- Update relevant documentation and samples when behaviour or APIs change.
- Add or update tests where they meaningfully protect the changed behaviour.
- Keep generated build output, local configuration, credentials, and downloaded
  runtime files out of the contribution.
- Preserve copyright and licence notices. Identify the source and licence of
  any third-party material you introduce.

## Checking your work

Choose checks that cover the affected component. The
[lint workflow](.github/workflows/lint.yml) lists repository validation and
Python contract tests; the [tests workflow](.github/workflows/tests.yml) builds
the supported native solution, managed plugin host, and C# samples.

Run relevant commands from the repository root after installing their
prerequisites. For example, GTK4 runtime staging changes should include:

```powershell
python tools\gtk4\test_stage_runtime.py
```

Build affected native or managed targets when changing their code. For
interface changes, check the behaviour in the Windows application and include
screenshots where useful. For installer or runtime changes, follow the staging
and package validation instructions in the GTK4 tools guide.

For documentation changes, check relative links, Markdown formatting, and the
accuracy of any commands. Before submitting, run `git diff --check` and review
the diff for unrelated files or sensitive information.

State which checks passed and which you could not run, with the reason.

## Submitting a pull request

Open a pull request against `main` with a clear title. Describe the problem,
the resulting behaviour, and how you verified it. Link the related issue,
include screenshots for visible changes, and call out compatibility changes
or known limitations. A draft pull request is useful for work that needs early
feedback.

Keep discussion constructive and focused on the change. Respond to review
feedback and update the description if the scope changes. Contributions are
subject to maintainer review; opening a pull request does not guarantee a merge.

Fabulor is licensed under the GNU General Public License version 3.0 only.
Contributions must be compatible with the [project licence](Licence.md).

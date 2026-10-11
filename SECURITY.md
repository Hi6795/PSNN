# Security policy

## Supported scope

The `main` development branch and released host-core versions are the current
review scope. **No PS4 firmware or title has been verified for deployment.**
The diagnostic shim is a host ABI experiment, not a Sony sign-in provider.

## Reporting a vulnerability

Do not publish exploit details, sensitive device logs, account credentials, or
console identifiers in a public issue. After GitHub publication, prefer a
**private vulnerability report through the repository Security tab**, if the
repository owner enables GitHub Private Vulnerability Reporting. Otherwise
contact the repository owner through an agreed private channel. No contact
address has been designated yet; do not invent one.

Include: affected revision, host/PS4 environment, reproducer stripped of
personal data, and impact. A project maintainer should validate the report
before it is disclosed publicly.

## Principles

- User opt-in; no hidden remote requests or persistent instrumentation.
- No Sony services, account tokens, credential handling, or entitlement bypass.
- Avoid privileged hooks in core; isolate adapter code per title and firmware.
- Preserve and never auto-delete failed/corrupt profile data.
- Treat binaries and third-party reverse-engineering artifacts as untrusted.

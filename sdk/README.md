# Aurea SDK

Public headers, runtime, linker script and packager for native Aurea OS applications (ABI version 1, Milestone 1).

```sh
make              # builds examples/* into out/
make APP=hello
```

Licensed under the MIT licence ([LICENSE](LICENSE)). Requires clang, ld.lld, ar, GNU make and Python 3. Read [docs/SDK.md](../docs/SDK.md) before using it: applications build correctly, but running them in Milestone 1 is affected by known kernel defects.

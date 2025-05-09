# Compiling with Native AOT

Please consult [documentation](https://learn.microsoft.com/dotnet/core/deploying/native-aot) for instructions how to compile and publish application.

The rest of this document covers advanced topics only.

## Using daily builds

For using daily builds, you need to make sure the `nuget.config` file for your project contains the following package sources under the `<packageSources>` element:
```xml
<add key="dotnet9" value="https://pkgs.dev.azure.com/dnceng/public/_packaging/dotnet9/nuget/v3/index.json" />
<add key="nuget" value="https://api.nuget.org/v3/index.json" />
```

If your project has no `nuget.config` file, it may be created by running
```bash
> dotnet new nugetconfig
```

from the project's root directory. New package sources must be added after the `<clear />` element if you decide to keep it.

Once you have added the package sources, add a reference to the ILCompiler package either by running
```bash
> dotnet add package Microsoft.DotNet.ILCompiler -v 10.0.0-*
```

or by adding the following element to the project file:
```xml
<ItemGroup>
  <PackageReference Include="Microsoft.DotNet.ILCompiler" Version="10.0.0-*" />
</ItemGroup>
```

Adding an explicit package reference to `Microsoft.DotNet.ILCompiler` will generate warning when publishing and it can run into version errors. When possible, use the default `Microsoft.DotNet.ILCompiler` version to publish a native AOT application.

## Cross-architecture compilation

Native AOT toolchain allows targeting ARM64 on an x64 host and vice versa for both Windows and Linux. Cross-OS compilation, such as targeting Linux on a Windows host, is not supported.

The target architecture can be specified using `-r` or `--arch` options of the `dotnet publish` command. For example, the following command produces Windows Arm64 binary on a Windows x64 host machine:

```bash
> dotnet publish -r win-arm64
```

The cross-architecture compilation requires native build tools for the target platform to be installed and configured correctly. On Linux, you may need to follow [cross-building instructions](../../../../docs/workflow/building/coreclr/cross-building.md) to create your own sysroot directory and specify path to it using the `SysRoot` property.

See [Building native AOT apps in containers](containers.md) for a streamlined path to establishing a Linux cross-compilation environments.

#### Using daily builds with cross-architecture compilation

For using daily builds according to the instructions above, in addition to the `Microsoft.DotNet.ILCompiler` package reference, also add the `runtime.win-x64.Microsoft.DotNet.ILCompiler` package reference to get the x64-hosted compiler:
```xml
<PackageReference Include="Microsoft.DotNet.ILCompiler; runtime.win-x64.Microsoft.DotNet.ILCompiler" Version="9.0.0-alpha.1.23456.7" />
```

Replace `9.0.0-alpha.1.23456.7` with the latest version from the [dotnet10](https://dev.azure.com/dnceng/public/_artifacts/feed/dotnet10/NuGet/Microsoft.DotNet.ILCompiler/) feed.
Note that it is important to use _the same version_ for both packages to avoid potential hard-to-debug issues. After adding the package reference, you may publish for win-arm64 as usual:
```bash
> dotnet publish -r win-arm64
```

Similarly, to target linux-arm64 on a Linux x64 host, in addition to the `Microsoft.DotNet.ILCompiler` package reference, also add the `runtime.linux-x64.Microsoft.DotNet.ILCompiler` package reference to get the x64-hosted compiler:
```xml
<PackageReference Include="Microsoft.DotNet.ILCompiler; runtime.linux-x64.Microsoft.DotNet.ILCompiler" Version="9.0.0-alpha.1.23456.7" />
```

## Using statically linked ICU
This feature can statically link libicu libraries (such as libicui18n.a) into your applications at build time.
NativeAOT binaries built with this feature can run even when libicu libraries are not installed.

You can use this feature by adding the `StaticICULinking` property to your project file as follows:

```xml
<PropertyGroup>
  <StaticICULinking>true</StaticICULinking>

  <!-- Optional: Embeds ICU data into the binary, making it fully standalone when system ICU
       libraries are not installed on the target machine.
       Update path to match your ICU version and variant: l(arge), b(ig endian), s(mall) or full. -->
  <EmbedIcuDataPath>/usr/share/icu/74.2/icudt74l.dat</EmbedIcuDataPath>
</PropertyGroup>
```

> [!NOTE]
> Some distros, such as Alpine and Gentoo, currently package ICU data as a `icudt*.dat` archive,
> while others, like Ubuntu, do not.
> To use `EmbedIcuDataPath` on a distro that does not provide the `.dat` file,
> you may need to build ICU with `--with-data-packaging=archive` to generate it.
> See https://unicode-org.github.io/icu/userguide/icu_data#building-and-linking-against-icu-data.
> ```sh
> # e.g. to obtain icudt*.dat on Ubuntu
> $ curl -sSL https://github.com/unicode-org/icu/releases/download/release-74-2/icu4c-74_2-src.tgz | tar xzf -
> $ cd icu/source
> $ ./configure --with-data-packaging=archive --enable-static --disable-shared --disable-samples
> $ make -j
> $ find . -path *out/* -name icudt*.dat -exec echo $(pwd)/{} \;
> ```

This feature is only supported on Linux. This feature is not supported when crosscompiling.

License (Unicode): https://github.com/unicode-org/icu/blob/main/icu4c/LICENSE

### Prerequisites

Ubuntu
```sh
apt install libicu-dev cmake
```

Alpine
```sh
apk add cmake icu-static icu-dev
```

## Using statically linked OpenSSL
This feature can statically link OpenSSL libraries (such as libssl.a and libcrypto.a) into your applications at build time.
NativeAOT binaries built with this feature can run even when OpenSSL libraries are not installed.
**WARNING:** *This is scenario for advanced users, please use with extreme caution. Incorrect usage of this feature, can cause security vulnerabilities in your product*

You can use this feature by adding the `StaticOpenSslLinking` property to your project file as follows:

```xml
<PropertyGroup>
  <StaticOpenSslLinking>true</StaticOpenSslLinking>
</PropertyGroup>
```

This feature is only supported on Linux. This feature is not supported when crosscompiling.

License for OpenSSL v3+ (Apache v2.0): https://github.com/openssl/openssl/blob/master/LICENSE.txt
License for OpenSSL releases prior to v3 (dual OpenSSL and SSLeay license): https://www.openssl.org/source/license-openssl-ssleay.txt

### Prerequisites

Ubuntu
```sh
apt install libssl-dev cmake
```

Alpine
```sh
apk add cmake openssl-dev openssl-libs-static
```

## NixOS
NativeAOT uses native executable `ilc` pulled from nuget, which has special requirements. Docs can be found at https://nixos.wiki/wiki/DotNET#NativeAOT

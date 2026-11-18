встановлюємо docker, wsl
запускаємо win застосунок

----
New-Item -ItemType Directory -Force -Path D:\v8-docker-build\work-dir

----
docker run --rm -v "D:\v8-docker-build\work-dir:/work" alpine sh -c "echo Docker-can-write-to-D > /work/test.txt && cat /work/test.txt"


----
@'
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    curl \
    git \
    python3 \
    xz-utils \
    file \
    build-essential \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /work
'@ | Set-Content -Encoding ascii D:\v8-docker-build\Dockerfile


----
docker build -t v8-android-builder D:\v8-docker-build

----
docker run --rm -v "D:\v8-docker-build\work-dir:/work" -w /work v8-android-builder git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git

----
docker run --rm -v "D:\v8-docker-build\work-dir:/work" -w /work/depot_tools v8-android-builder bash -lc 'export PATH="$PWD:$PATH"; gclient --version'

----
docker run --rm v8-android-builder git ls-remote --tags https://chromium.googlesource.com/v8/v8.git "refs/tags/12.4.254.21"

----
docker run --rm -v "D:\v8-docker-build\work-dir:/work" -w /work v8-android-builder bash -lc 'cd /work/v8 && git fetch --depth=1 origin refs/tags/15.8.21:refs/tags/15.8.21 && git checkout 15.8.21 && git describe --tags --exact-match'

----
docker run --rm -v "D:\v8-docker-build\work-dir:/work" -w /work v8-android-builder bash -lc 'export PATH="/work/depot_tools:$PATH"; cd /work/v8; gclient sync --nohooks --no-history -j 2 --verbose'


----
docker run --rm -v "D:\v8-docker-build\work-dir:/work" -w /work/v8 v8-android-builder bash -lc "git describe --tags --exact-match && git status --short --branch"


----
Add-Content -Path "D:\v8-docker-build\work-dir\.gclient" -Value "`ntarget_os = ['android']"

----
docker run --rm -v "D:\v8-docker-build\work-dir:/work" -w /work/v8 v8-android-builder bash -lc 'export PATH="/work/depot_tools:$PATH"; gclient sync --nohooks --no-history -j 2'

----
docker run --rm -v "D:\v8-docker-build\work-dir:/work" -w /work/v8 v8-android-builder bash -c './buildtools/linux64/gn gen out/arm64.release --args='\''target_os="android" target_cpu="arm64" v8_target_cpu="arm64" is_debug=false is_component_build=false v8_monolithic=true v8_use_external_startup_data=false'\'''

----
@'
target_os = "android"
target_cpu = "arm64"
v8_target_cpu = "arm64"
is_debug = false
is_component_build = false
v8_monolithic = true
v8_use_external_startup_data = false
use_custom_libcxx = false
use_custom_libcxx_for_host = true
v8_enable_sandbox = false
v8_enable_temporal_support = false
symbol_level = 0
v8_symbol_level = 0
v8_enable_i18n_support = false
v8_enable_webassembly = false
android64_ndk_api_level = 27
'@ | Set-Content "D:\v8-docker-build\work-dir\args.arm64.gn"

----
docker run --rm -v "D:\v8-docker-build\work-dir:/work" -w /work/v8 v8-android-builder bash -c "cp /work/args.arm64.gn out/arm64.ndk-libcxx/args.gn && ./buildtools/linux64/gn gen out/arm64.ndk-libcxx"


----
docker run --rm -v "D:\v8-docker-build\work-dir:/work" -w /work/v8 v8-android-builder ./third_party/ninja/ninja -C out/arm64.ndk-libcxx -j 2 v8_monolith

----

@'
target_os = "android"
target_cpu = "x64"
v8_target_cpu = "x64"
is_debug = false
is_component_build = false
v8_monolithic = true
v8_use_external_startup_data = false
use_custom_libcxx = false
use_custom_libcxx_for_host = true
v8_enable_sandbox = false
v8_enable_temporal_support = false
symbol_level = 0
v8_symbol_level = 0
v8_enable_i18n_support = false
v8_enable_webassembly = false
'@ | Set-Content "D:\v8-docker-build\work-dir\args.x64.gn"


----
docker run --rm -v "D:\v8-docker-build\work-dir:/work" -w /work/v8 v8-android-builder bash -c "mkdir -p out/x64.ndk-libcxx && cp /work/args.x64.gn out/x64.ndk-libcxx/args.gn && ./buildtools/linux64/gn gen out/x64.ndk-libcxx"


----
docker run --rm -v "D:\v8-docker-build\work-dir:/work" -w /work/v8 v8-android-builder ./third_party/ninja/ninja -C out/x64.ndk-libcxx -j 2 v8_monolith




----
windows
open "x64 Native Tools Command Prompt for VS 2022"

cd /d D:\
git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git D:\depot_tools

set "PATH=D:\depot_tools;%PATH%"
set DEPOT_TOOLS_WIN_TOOLCHAIN=0
set "PATH=D:\depot_tools;%PATH%"
set DEPOT_TOOLS_WIN_TOOLCHAIN=0
set "vs2022_install=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools"
set "GYP_MSVS_OVERRIDE_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools"
set GYP_MSVS_VERSION=2022
set "WindowsSDKVersion=10.0.26100.0\"


gclient

cd /d D:\
mkdir v8-windows
cd v8-windows



fetch v8
git -C D:\v8-windows\v8 fetch origin refs/tags/12.4.254.21:refs/tags/12.4.254.21
git -C D:\v8-windows\v8 checkout 12.4.254.21


cd /d D:\v8-windows\v8
gclient sync


open D:\v8-windows\v8\build\toolchain\win\setup_toolchain.py - SDK_VERSION = '10.0.26100.0'
D:\v8-windows\v8\build\vs_toolchain.py - SDK_VERSION = '10.0.26100.0'

gn args out.gn\x64.release      <and paste:>

is_debug = false
target_cpu = "x64"
is_component_build = false
v8_monolithic = true
v8_use_external_startup_data = false
v8_enable_sandbox = false
v8_enable_temporal_support = false
v8_enable_i18n_support = false
v8_enable_webassembly = false
symbol_level = 0
v8_symbol_level = 0
use_custom_libcxx = false
is_official_build = false
chrome_pgo_phase = 0 # вимикає PGO-оптимізацію Chromium, для якої у нас нема залежностей
optimize_for_size = true
v8_enable_disassembler = false # Вбудований дисасемблер
v8_enable_gdbjit = false # Інтеграція з GDB
v8_use_perfetto = false # Трасування Perfetto
v8_postmortem_support = false # Постмортем метадані

autoninja -C out.gn\x64.release -t clean
autoninja -C out.gn\x64.release v8_monolith






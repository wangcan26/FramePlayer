base_path=$(cd `dirname $0`; pwd)

## requirements:
### cmake
### ninja

platform='unknown'
case $OSTYPE in 
    msys*) platform='windows';;
    darwin*) platform='darwin';;
esac
echo "current platform : ${platform}"

source $base_path/VERSION.sh
if [ "$platform" = "windows" ]; then 
    export ANDROID_NDK=$ANDROID_NDK_VERSION_WIN
fi
echo "use ndk": $ANDROID_NDK
if [[ -z "${ANDROID_NDK}" || ! -f $ANDROID_NDK/build/cmake/android.toolchain.cmake ]]; then
  echo "ANDROID_NDK is undefined or not valid"
  exit 1
else
  echo "ANDROID_NDK=${ANDROID_NDK}"
fi

function build_frameplayer_lib_for_abi() {
    echo "#********* build frame player ******/"

    local abs_path_build=$base_path/../cmake_build/$2
    # echo $abs_path
    if [ ! -d "$abs_path_build" ]; then
    #   echo "mkdir: cmake_build"
      mkdir -p $abs_path_build
    fi

    # exit 0
    cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
        -DCMAKE_BUILD_TYPE=$2 \
        -DANDROID_ABI=$1 \
        -DANDROID_PLATFORM=android-28 \
        -G Ninja \
        -B $abs_path_build -S $base_path/../ || exit 1
         \

    cmake --build $abs_path_build
}

function copy_frameplayer_lib_to_app() {
  local abi_type=$1
  local build_type=$2
  local src_path_file=$base_path/../cmake_build/${build_type}/source/main/libfpn_mono.so 
  local target_path_folder=$base_path/../../app/src/main/jniLibs/${abi_type}
  if [ ! -f "$src_path_file" ]; then
    echo "copy failed: " $src_path_file "does not exist"
    return
  fi
  if [ ! -d "$target_path_folder" ]; then
    mkdir -p $target_path_folder
  fi
  echo "installing fpn_main.so...."
  cp  -rf $src_path_file $target_path_folder
  echo "install scucess"
}

build_frameplayer_lib_for_abi arm64-v8a Release
copy_frameplayer_lib_to_app arm64-v8a Release

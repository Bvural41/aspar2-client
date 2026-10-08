cmake_minimum_required(VERSION 3.10.2)
project("Metin2Mobile" C CXX)

if(APPLE)
    add_definitions(-D__APPLE__)
    add_definitions(-D__IOS__)
    add_definitions(-D_DARWIN_C_SOURCE)
elseif(ANDROID)
    add_definitions(-D__ANDROID__)
    add_definitions(-D_GNU_SOURCE)
endif()

add_definitions(-DUSE_OPENGL_ES)
add_definitions(-DENABLE_OPENGL)
add_definitions(-DENABLE_MOBILE_CONTROLS)
add_definitions(-DLOCALE_SERVICE_EUROPE)
add_definitions(-DNDEBUG)

# String and POSIX compatibility defines
add_definitions(-D_snprintf=snprintf)
add_definitions(-D_vsnprintf=vsnprintf)
add_definitions(-D_stricmp=strcasecmp)
add_definitions(-D_strnicmp=strncasecmp)
add_definitions(-Dstricmp=strcasecmp)
add_definitions(-Dstrnicmp=strncasecmp)
add_definitions(-Dstrcmpi=strcasecmp)
add_definitions(-D_strcmpi=strcasecmp)
add_definitions(-D_access=access)

# Crypto++ mobile defines
add_definitions(-DCRYPTOPP_DISABLE_ASM)
add_definitions(-DCRYPTOPP_DISABLE_X86ASM)
add_definitions(-DCRYPTOPP_DISABLE_SSE2)
add_definitions(-DCRYPTOPP_DISABLE_SSSE3)

if(ANDROID OR APPLE)
    add_definitions(-DGRANNY_ONLY_TYPES)
endif()

add_compile_options(
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++17>
    $<$<COMPILE_LANGUAGE:CXX>:-frtti>
    $<$<COMPILE_LANGUAGE:CXX>:-fexceptions>
    -O3
    -fomit-frame-pointer
    -fdeclspec
    -fms-extensions
    -fno-strict-aliasing
    -Wno-register
    -Wno-invalid-source-encoding
    -Wno-address-of-temporary
    -Wno-format-security
    -Wno-narrowing
    -Wno-return-type
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/Python-2.7.18/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/Python-2.7.18/Modules/_io
    ${CMAKE_CURRENT_SOURCE_DIR}/android_compat
    ${CMAKE_CURRENT_SOURCE_DIR}/source
    ${CMAKE_CURRENT_SOURCE_DIR}/source/common
    ${CMAKE_CURRENT_SOURCE_DIR}/source/EterBase
    ${CMAKE_CURRENT_SOURCE_DIR}/source/EterBase/Poly
    ${CMAKE_CURRENT_SOURCE_DIR}/source/EterLib
    ${CMAKE_CURRENT_SOURCE_DIR}/source/EterPythonLib
    ${CMAKE_CURRENT_SOURCE_DIR}/source/GameLib
    ${CMAKE_CURRENT_SOURCE_DIR}/source/UserInterface
    ${CMAKE_CURRENT_SOURCE_DIR}/source/MilesLib
    ${CMAKE_CURRENT_SOURCE_DIR}/source/EffectLib
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SphereLib
    ${CMAKE_CURRENT_SOURCE_DIR}/source/EterPack
    ${CMAKE_CURRENT_SOURCE_DIR}/source/EterGrnLib
    ${CMAKE_CURRENT_SOURCE_DIR}/source/EterImageLib
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SpeedTreeLib
    ${CMAKE_CURRENT_SOURCE_DIR}/source/PRTerrainLib
    ${CMAKE_CURRENT_SOURCE_DIR}/source/ScriptLib
    ${CMAKE_CURRENT_SOURCE_DIR}/source/EterLocale
    ${CMAKE_CURRENT_SOURCE_DIR}/source/CWebBrowser
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/GrannySource/include
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/GrannySource/source
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/include
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/include/Python-2.7
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lzo
)

if(ANDROID)
    include_directories(${ANDROID_NDK}/sources/android/cpufeatures)
elseif(APPLE)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ios)
endif()

file(GLOB ETER_BASE_SOURCES source/EterBase/*.cpp source/EterBase/Poly/*.cpp)
file(GLOB ETER_LIB_SOURCES source/EterLib/*.cpp)
file(GLOB ETER_IMAGE_LIB_SOURCES source/EterImageLib/*.cpp)
file(GLOB GAME_LIB_SOURCES source/GameLib/*.cpp)
file(GLOB USER_INTERFACE_SOURCES source/UserInterface/*.cpp)
if(APPLE)
    list(FILTER USER_INTERFACE_SOURCES EXCLUDE REGEX ".*(UserInterface\\.cpp|AndroidMain\\.cpp|Cef|HackShield|NProtect|WiseLogicXTrap|Process|MovieMan|CheckLatestFiles|Logo|PythonTitleSystem).*")
else()
    list(FILTER USER_INTERFACE_SOURCES EXCLUDE REGEX ".*(UserInterface\\.cpp|Cef|HackShield|NProtect|WiseLogicXTrap|Process|MovieMan|CheckLatestFiles|Logo|PythonTitleSystem).*")
endif()
# Android touch controls included in USER_INTERFACE_SOURCES

file(GLOB ETER_PACK_SOURCES source/EterPack/*.cpp source/EterPack/*.c)
file(GLOB ETER_GRN_LIB_SOURCES source/EterGrnLib/*.cpp)
file(GLOB EFFECT_LIB_SOURCES source/EffectLib/*.cpp)
file(GLOB ETER_PYTHON_LIB_SOURCES source/EterPythonLib/*.cpp)
file(GLOB SCRIPT_LIB_SOURCES source/ScriptLib/*.cpp)
file(GLOB SPHERE_LIB_SOURCES source/SphereLib/*.cpp)
file(GLOB MILES_LIB_SOURCES source/MilesLib/*.cpp)
file(GLOB SPEED_TREE_LIB_SOURCES source/SpeedTreeLib/*.cpp)
file(GLOB PR_TERRAIN_LIB_SOURCES source/PRTerrainLib/*.cpp)
file(GLOB ETER_LOCALE_SOURCES source/EterLocale/*.cpp)

set(LZO_SOURCES
    source/lzo/lzo1x_1.c
    source/lzo/lzo1x_9x.c
    source/lzo/lzo1x_d1.c
    source/lzo/lzo1x_d2.c
    source/lzo/lzo_ptr.c
    source/lzo/lzo_str.c
    source/lzo/lzo_util.c
    source/lzo/lzo_crc.c
)

file(GLOB CRYPTOPP_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/include/cryptopp/*.cpp
)
list(FILTER CRYPTOPP_SOURCES EXCLUDE REGEX ".*(test|validat|bench|adhoc|fipsalgt|fipstest).*")

set(PYTHON_SOURCES
    extern/Python-2.7.18/Parser/acceler.c
    extern/Python-2.7.18/Parser/bitset.c
    extern/Python-2.7.18/Parser/firstsets.c
    extern/Python-2.7.18/Parser/grammar.c
    extern/Python-2.7.18/Parser/grammar1.c
    extern/Python-2.7.18/Parser/listnode.c
    extern/Python-2.7.18/Parser/metagrammar.c
    extern/Python-2.7.18/Parser/myreadline.c
    extern/Python-2.7.18/Parser/node.c
    extern/Python-2.7.18/Parser/parser.c
    extern/Python-2.7.18/Parser/parsetok.c
    extern/Python-2.7.18/Parser/pgen.c
    extern/Python-2.7.18/Parser/tokenizer.c

    extern/Python-2.7.18/Objects/abstract.c
    extern/Python-2.7.18/Objects/boolobject.c
    extern/Python-2.7.18/Objects/bufferobject.c
    extern/Python-2.7.18/Objects/bytes_methods.c
    extern/Python-2.7.18/Objects/bytearrayobject.c
    extern/Python-2.7.18/Objects/capsule.c
    extern/Python-2.7.18/Objects/cellobject.c
    extern/Python-2.7.18/Objects/classobject.c
    extern/Python-2.7.18/Objects/cobject.c
    extern/Python-2.7.18/Objects/codeobject.c
    extern/Python-2.7.18/Objects/complexobject.c
    extern/Python-2.7.18/Objects/descrobject.c
    extern/Python-2.7.18/Objects/enumobject.c
    extern/Python-2.7.18/Objects/exceptions.c
    extern/Python-2.7.18/Objects/genobject.c
    extern/Python-2.7.18/Objects/fileobject.c
    extern/Python-2.7.18/Objects/floatobject.c
    extern/Python-2.7.18/Objects/frameobject.c
    extern/Python-2.7.18/Objects/funcobject.c
    extern/Python-2.7.18/Objects/intobject.c
    extern/Python-2.7.18/Objects/iterobject.c
    extern/Python-2.7.18/Objects/listobject.c
    extern/Python-2.7.18/Objects/longobject.c
    extern/Python-2.7.18/Objects/dictobject.c
    extern/Python-2.7.18/Objects/memoryobject.c
    extern/Python-2.7.18/Objects/methodobject.c
    extern/Python-2.7.18/Objects/moduleobject.c
    extern/Python-2.7.18/Objects/object.c
    extern/Python-2.7.18/Objects/obmalloc.c
    extern/Python-2.7.18/Objects/rangeobject.c
    extern/Python-2.7.18/Objects/setobject.c
    extern/Python-2.7.18/Objects/sliceobject.c
    extern/Python-2.7.18/Objects/stringobject.c
    extern/Python-2.7.18/Objects/structseq.c
    extern/Python-2.7.18/Objects/tupleobject.c
    extern/Python-2.7.18/Objects/typeobject.c
    extern/Python-2.7.18/Objects/weakrefobject.c
    extern/Python-2.7.18/Objects/unicodeobject.c
    extern/Python-2.7.18/Objects/unicodectype.c

    extern/Python-2.7.18/Python/_warnings.c
    extern/Python-2.7.18/Python/Python-ast.c
    extern/Python-2.7.18/Python/asdl.c
    extern/Python-2.7.18/Python/ast.c
    extern/Python-2.7.18/Python/bltinmodule.c
    extern/Python-2.7.18/Python/ceval.c
    extern/Python-2.7.18/Python/compile.c
    extern/Python-2.7.18/Python/codecs.c
    extern/Python-2.7.18/Python/errors.c
    extern/Python-2.7.18/Python/frozen.c
    extern/Python-2.7.18/Python/frozenmain.c
    extern/Python-2.7.18/Python/future.c
    extern/Python-2.7.18/Python/getargs.c
    extern/Python-2.7.18/Python/getcompiler.c
    extern/Python-2.7.18/Python/getcopyright.c
    extern/Python-2.7.18/Python/getplatform.c
    extern/Python-2.7.18/Python/getversion.c
    extern/Python-2.7.18/Python/graminit.c
    extern/Python-2.7.18/Python/import.c
    extern/Python-2.7.18/Python/importdl.c
    extern/Python-2.7.18/Python/marshal.c
    extern/Python-2.7.18/Python/modsupport.c
    extern/Python-2.7.18/Python/mystrtoul.c
    extern/Python-2.7.18/Python/mysnprintf.c
    extern/Python-2.7.18/Python/peephole.c
    extern/Python-2.7.18/Python/pyarena.c
    extern/Python-2.7.18/Python/pyctype.c
    extern/Python-2.7.18/Python/pyfpe.c
    extern/Python-2.7.18/Python/pymath.c
    extern/Python-2.7.18/Python/pystate.c
    extern/Python-2.7.18/Python/pythonrun.c
    extern/Python-2.7.18/Python/random.c
    extern/Python-2.7.18/Python/structmember.c
    extern/Python-2.7.18/Python/symtable.c
    extern/Python-2.7.18/Python/sysmodule.c
    extern/Python-2.7.18/Python/traceback.c
    extern/Python-2.7.18/Python/getopt.c
    extern/Python-2.7.18/Python/pystrcmp.c
    extern/Python-2.7.18/Python/pystrtod.c
    extern/Python-2.7.18/Python/dtoa.c
    extern/Python-2.7.18/Python/formatter_unicode.c
    extern/Python-2.7.18/Python/formatter_string.c
    extern/Python-2.7.18/Python/dynload_shlib.c
    extern/Python-2.7.18/Python/thread.c

    extern/Python-2.7.18/Modules/gcmodule.c
    extern/Python-2.7.18/Modules/getpath.c
    extern/Python-2.7.18/Modules/config.c
    extern/Python-2.7.18/Modules/getbuildinfo.c
    extern/Python-2.7.18/Modules/threadmodule.c
    extern/Python-2.7.18/Modules/signalmodule.c
    extern/Python-2.7.18/Modules/posixmodule.c
    extern/Python-2.7.18/Modules/errnomodule.c
    extern/Python-2.7.18/Modules/pwdmodule.c
    extern/Python-2.7.18/Modules/_sre.c
    extern/Python-2.7.18/Modules/_codecsmodule.c
    extern/Python-2.7.18/Modules/_weakref.c
    extern/Python-2.7.18/Modules/zipimport.c
    extern/Python-2.7.18/Modules/symtablemodule.c
    extern/Python-2.7.18/Modules/main.c
    extern/Python-2.7.18/Modules/arraymodule.c
    extern/Python-2.7.18/Modules/cmathmodule.c
    extern/Python-2.7.18/Modules/_math.c
    extern/Python-2.7.18/Modules/mathmodule.c
    extern/Python-2.7.18/Modules/_struct.c
    extern/Python-2.7.18/Modules/timemodule.c
    extern/Python-2.7.18/Modules/operator.c
    extern/Python-2.7.18/Modules/_randommodule.c
    extern/Python-2.7.18/Modules/_collectionsmodule.c
    extern/Python-2.7.18/Modules/_heapqmodule.c
    extern/Python-2.7.18/Modules/itertoolsmodule.c
    extern/Python-2.7.18/Modules/stropmodule.c
    extern/Python-2.7.18/Modules/_functoolsmodule.c
    extern/Python-2.7.18/Modules/datetimemodule.c
    extern/Python-2.7.18/Modules/_bisectmodule.c
    extern/Python-2.7.18/Modules/unicodedata.c
    extern/Python-2.7.18/Modules/_io/bufferedio.c
    extern/Python-2.7.18/Modules/_io/bytesio.c
    extern/Python-2.7.18/Modules/_io/fileio.c
    extern/Python-2.7.18/Modules/_io/iobase.c
    extern/Python-2.7.18/Modules/_io/_iomodule.c
    extern/Python-2.7.18/Modules/_io/stringio.c
    extern/Python-2.7.18/Modules/_io/textio.c
    extern/Python-2.7.18/Modules/fcntlmodule.c
    extern/Python-2.7.18/Modules/selectmodule.c
    extern/Python-2.7.18/Modules/mmapmodule.c
    extern/Python-2.7.18/Modules/_csv.c
    extern/Python-2.7.18/Modules/socketmodule.c
    extern/Python-2.7.18/Modules/termios.c
    extern/Python-2.7.18/Modules/md5module.c
    extern/Python-2.7.18/Modules/md5.c
    extern/Python-2.7.18/Modules/shamodule.c
    extern/Python-2.7.18/Modules/sha256module.c
    extern/Python-2.7.18/Modules/sha512module.c
    extern/Python-2.7.18/Modules/binascii.c
    extern/Python-2.7.18/Modules/parsermodule.c
    extern/Python-2.7.18/Modules/cStringIO.c
    extern/Python-2.7.18/Modules/cPickle.c
    extern/Python-2.7.18/Modules/xxsubtype.c
    extern/Python-2.7.18/Modules/future_builtins.c
    extern/Python-2.7.18/Modules/_json.c
    extern/Python-2.7.18/Modules/zlibmodule.c
)
if(APPLE)
    set_source_files_properties(${PYTHON_SOURCES} PROPERTIES COMPILE_DEFINITIONS "Py_BUILD_CORE;PLATFORM=\"darwin\";DUSE_ZLIB_CRC32")
else()
    set_source_files_properties(${PYTHON_SOURCES} PROPERTIES COMPILE_DEFINITIONS "Py_BUILD_CORE;PLATFORM=\"linux2\";DUSE_ZLIB_CRC32")
endif()

file(GLOB GRANNY_CPP_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/GrannySource/source/granny_*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/GrannySource/source/rr*.cpp
)

set(GRANNY_C_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/GrannySource/source/arithbit.c
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/GrannySource/source/binktc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/GrannySource/source/encode.c
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/GrannySource/source/radarith.c
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/GrannySource/source/radlz.c
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/GrannySource/source/rrCore.c
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/GrannySource/source/varbits.c
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/GrannySource/source/wavelet.c
)

set(GRANNY_PLATFORM_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/android_compat/android_granny_platform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/GrannySource/source/winxx/win64/granny.cpp
)

set(GRANNY_ALL_SOURCES
    ${GRANNY_CPP_SOURCES}
    ${GRANNY_C_SOURCES}
    ${GRANNY_PLATFORM_SOURCES}
)
set_source_files_properties(${GRANNY_ALL_SOURCES} PROPERTIES COMPILE_DEFINITIONS "BUILDING_GRANNY_STATIC=1;BUILDING_GRANNY=1;RAD_NO_LOWERCASE_TYPES=0;_HAS_ITERATOR_DEBUGGING=0")

if(ANDROID)
    add_library(metin2_mobile SHARED
        ${USER_INTERFACE_SOURCES}
        ${ETER_BASE_SOURCES}
        ${ETER_LIB_SOURCES}
        ${ETER_IMAGE_LIB_SOURCES}
        ${GAME_LIB_SOURCES}
        ${ETER_PACK_SOURCES}
        ${ETER_GRN_LIB_SOURCES}
        ${EFFECT_LIB_SOURCES}
        ${ETER_PYTHON_LIB_SOURCES}
        ${SCRIPT_LIB_SOURCES}
        ${SPHERE_LIB_SOURCES}
        ${MILES_LIB_SOURCES}
        ${SPEED_TREE_LIB_SOURCES}
        ${PR_TERRAIN_LIB_SOURCES}
        ${ETER_LOCALE_SOURCES}
        ${CRYPTOPP_SOURCES}
        ${LZO_SOURCES}
        ${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c
        ${CMAKE_CURRENT_SOURCE_DIR}/android_compat/d3dx9math_impl.cpp
        ${PYTHON_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/android_compat/android_engine_stubs.cpp
        ${GRANNY_ALL_SOURCES}
    )

    find_library(log-lib log)
    find_library(android-lib android)
    find_library(gles3-lib GLESv3)

    target_link_libraries(metin2_mobile
        ${log-lib}
        ${android-lib}
        ${gles3-lib}
        z
        m
    )

    target_link_options(metin2_mobile PRIVATE
        "-Wl,--no-dependent-libraries"
        "-Wl,--no-undefined"
        "-Wl,--error-limit=0"
    )
elseif(APPLE)
    enable_language(OBJC OBJCXX)

    file(GLOB IOS_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/ios/*.mm
        ${CMAKE_CURRENT_SOURCE_DIR}/ios/*.m
        ${CMAKE_CURRENT_SOURCE_DIR}/ios/*.h
    )

    add_executable(Aspar2 MACOSX_BUNDLE
        ${USER_INTERFACE_SOURCES}
        ${IOS_SOURCES}
        ${ETER_BASE_SOURCES}
        ${ETER_LIB_SOURCES}
        ${ETER_IMAGE_LIB_SOURCES}
        ${GAME_LIB_SOURCES}
        ${ETER_PACK_SOURCES}
        ${ETER_GRN_LIB_SOURCES}
        ${EFFECT_LIB_SOURCES}
        ${ETER_PYTHON_LIB_SOURCES}
        ${SCRIPT_LIB_SOURCES}
        ${SPHERE_LIB_SOURCES}
        ${MILES_LIB_SOURCES}
        ${SPEED_TREE_LIB_SOURCES}
        ${PR_TERRAIN_LIB_SOURCES}
        ${ETER_LOCALE_SOURCES}
        ${CRYPTOPP_SOURCES}
        ${LZO_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/android_compat/d3dx9math_impl.cpp
        ${PYTHON_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/android_compat/android_engine_stubs.cpp
        ${GRANNY_ALL_SOURCES}
    )

    target_link_libraries(Aspar2
        "-framework Foundation"
        "-framework UIKit"
        "-framework OpenGLES"
        "-framework CoreGraphics"
        "-framework QuartzCore"
        "-framework AudioToolbox"
        "-framework CoreAudio"
        "-framework WebKit"
        "-framework AVFoundation"
        z
        m
    )

    set_target_properties(Aspar2 PROPERTIES
        MACOSX_BUNDLE YES
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/ios/Info.plist
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.aspar2.client"
        XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"
        XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC "YES"
        XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "14.0"
        XCODE_ATTRIBUTE_ENABLE_BITCODE "NO"
    )
endif()

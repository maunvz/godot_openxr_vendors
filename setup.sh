git submodule update --init
cd thirdparty/godot-cpp
scons platform=android target=template_debug arch=arm64
scons platform=android target=template_debug arch=arm32
scons platform=android target=template_debug arch=x86_64
scons platform=android target=template_debug arch=x86_32
scons platform=android target=template_release arch=arm64
scons platform=android target=template_release arch=arm32
scons platform=android target=template_release arch=x86_64
scons platform=android target=template_release arch=x86_32

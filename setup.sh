# Exit on any failure
set -e

# Update both API source files
cd thirdparty/godot_cpp_gdextension_api/
godot --dump-extension-api --headless
cd ../godot-cpp/gdextension/
godot --dump-extension-api --dump-gdextension-interface --headless
cd ../

# Build everything
scons platform=android target=template_debug arch=arm64 custom_api_file=../godot_cpp_gdextension_api/extension_api.json
scons platform=android target=template_release arch=arm64 custom_api_file=../godot_cpp_gdextension_api/extension_api.json
scons platform=android target=template_debug arch=x86_64 custom_api_file=../godot_cpp_gdextension_api/extension_api.json
scons platform=android target=template_release arch=x86_64 custom_api_file=../godot_cpp_gdextension_api/extension_api.json

# Return
cd ../../
./clean.sh

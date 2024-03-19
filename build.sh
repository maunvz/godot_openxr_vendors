# Exit on any failure
set -e

scons target=template_debug
scons target=template_release
scons target=template_debug platform=android
scons target=template_release platform=android
./gradlew build

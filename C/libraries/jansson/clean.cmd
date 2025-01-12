set build_dir_name=builds
set dir_to_delete=%CD%\%build_dir_name%

echo You are about to permanently delete everything in %dir_to_delete%?

rmdir /s %dir_to_delete%
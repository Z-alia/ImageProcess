set(bundle_root "C:/Users/28693/Desktop/Smart-Car-dx/udp/ImageProcess/build/dist/imageprocessor")
set(executable_path "C:/Users/28693/Desktop/Smart-Car-dx/udp/ImageProcess/build/bin/imageprocessor.exe")
set(runtime_search_dirs "C:/msys64/mingw64/bin/../bin")
set(data_dir_mappings "C:/msys64/mingw64/bin/../share/glib-2.0|share;C:/msys64/mingw64/bin/../share/icons|share;C:/msys64/mingw64/bin/../share/themes|share;C:/msys64/mingw64/bin/../etc|.;C:/msys64/mingw64/bin/../lib/gdk-pixbuf-2.0|lib;C:/msys64/mingw64/bin/../lib/gtk-3.0|lib")
set(extra_files "C:/Users/28693/Desktop/Smart-Car-dx/udp/ImageProcess/run_imageprocessor.bat;C:/Users/28693/Desktop/Smart-Car-dx/udp/ImageProcess/README.md")

if(NOT EXISTS "${executable_path}")
    message(FATAL_ERROR "Executable not found: ${executable_path}")
endif()

file(MAKE_DIRECTORY "${bundle_root}/bin")
file(COPY "${executable_path}" DESTINATION "${bundle_root}/bin")

set(runtime_deps "")
set(unresolved_deps "")
if(runtime_search_dirs)
    file(GET_RUNTIME_DEPENDENCIES
        RESOLVED_DEPENDENCIES_VAR runtime_deps
        UNRESOLVED_DEPENDENCIES_VAR unresolved_deps
        EXECUTABLES "${executable_path}"
        DIRECTORIES ${runtime_search_dirs}
    )
else()
    file(GET_RUNTIME_DEPENDENCIES
        RESOLVED_DEPENDENCIES_VAR runtime_deps
        UNRESOLVED_DEPENDENCIES_VAR unresolved_deps
        EXECUTABLES "${executable_path}"
    )
endif()

foreach(dep IN LISTS runtime_deps)
    if(EXISTS "${dep}")
        get_filename_component(dep_name "${dep}" NAME)
        if(NOT EXISTS "${bundle_root}/bin/${dep_name}")
            file(COPY "${dep}" DESTINATION "${bundle_root}/bin")
        endif()
    endif()
endforeach()

if(unresolved_deps)
    message(WARNING "Unresolved runtime dependencies: ${unresolved_deps}")
endif()

foreach(mapping IN LISTS data_dir_mappings)
    if(mapping STREQUAL "")
        continue()
    endif()
    string(REPLACE "|" ";" mapping_parts "${mapping}")
    list(LENGTH mapping_parts mapping_len)
    if(mapping_len LESS 1)
        continue()
    endif()
    list(GET mapping_parts 0 source_dir)
    if(mapping_len GREATER 1)
        list(GET mapping_parts 1 dest_parent)
    else()
        set(dest_parent ".")
    endif()
    if(NOT EXISTS "${source_dir}")
        continue()
    endif()
    if(dest_parent STREQUAL "." OR dest_parent STREQUAL "")
        set(dest_dir "${bundle_root}")
    else()
        set(dest_dir "${bundle_root}/${dest_parent}")
    endif()
    file(MAKE_DIRECTORY "${dest_dir}")
    file(COPY "${source_dir}" DESTINATION "${dest_dir}")
endforeach()

foreach(extra IN LISTS extra_files)
    if(EXISTS "${extra}")
        file(COPY "${extra}" DESTINATION "${bundle_root}")
    endif()
endforeach()

set(portable_readme "${bundle_root}/README-portable.txt")
file(WRITE "${portable_readme}" "ImageProcessor Portable Build\n==============================\n\nContents:\n- bin\\imageprocessor.exe (application + required DLLs)\n- share/, lib/, etc/ runtime resources copied from GTK/MSYS2\n- run_imageprocessor.bat launcher\n\nUsage:\n1. Double-click run_imageprocessor.bat (ensures required environment variables).\n2. Optionally invoke 'bin\\imageprocessor.exe' manually after setting PATH to this folder.\n\nThis bundle was generated on 2025-10-20 17:33:58.\n")

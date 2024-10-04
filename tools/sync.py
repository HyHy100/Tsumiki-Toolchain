import subprocess

def fetch_binaries():
    # TODO: Fetch all the required binaries here.
    # For example:
    #   Ninja
    #   CMake
    #   Clang
    print('todo')
    

def update_submodules():
    subprocess.run(['git', 'submodule', 'update', '--init', '.'], cwd='src/third_party/khronos/vulkan-hpp/tinyxml2')
    subprocess.run(['git', 'submodule', 'update', '--init', '.'], cwd='src/third_party/khronos/vulkan-hpp/Vulkan-Headers')

fetch_binaries()
update_submodules()
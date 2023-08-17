
import os, subprocess, shutil

print_mode = False

class ShaderCompiler:
  def __init__(self, shader_dir, glslcompiler_path):
    self.shader_dir = shader_dir
    self.spv_dir = self.shader_dir + '/spv/'
    self.glsl_compiler_path = glslcompiler_path
    
  def compile(self):
    print('Compiling shaders...')
    shader_files = os.listdir(self.shader_dir)
    for file in shader_files:
      if '.vert' in file or '.frag' in file:
        postfix = 'Vert'
        if '.frag' in file:
          postfix = 'Frag'
      
        dstfile = self.spv_dir+file
        if '.vert' in dstfile:
          dstfile = dstfile.replace('.vert', 'Vert.spv')
        if '.frag' in dstfile:
          dstfile = dstfile.replace('.frag', 'Frag.spv')
        
        srcfile = self.shader_dir + '/' + file
        if print_mode:
          print(self.glsl_compiler_path + ' ' + srcfile + ' -o ' + dstfile)
        else:
          subprocess.call([self.glsl_compiler_path, srcfile, '-o', dstfile] )
          print('Completed compiling', file)
    print('\n')
        


class ArtifactCopier:
  def __init__(self, resource_dirs, target_dir, dst_sub_dir, ext=''):
    print('Copying resources to build folder...')
    #make the folder if it doesnt exist
    if len(dst_sub_dir) != 0:
      for sub_folder in dst_sub_dir:
        final_dst_dir = target_dir + '/'+sub_folder
        if not os.path.exists(final_dst_dir):
          os.makedirs(final_dst_dir)
    
    # copy the folders
    for idx, src_folder in enumerate(resource_dirs):
      src_files = os.listdir(src_folder)
      for src_file in src_files:
        if len(dst_sub_dir) != 0:
          final_dst_file = target_dir+'/'+dst_sub_dir[idx]+'/'+src_file
        else:
          final_dst_file = target_dir+'/'+src_file
          
        final_src_file = src_folder + '/' + src_file
        if print_mode:
          print('Copy '+final_src_file+' to '+final_dst_file)
        else:
          if len(ext) != 0 and ext in src_file:
            shutil.copy(final_src_file, final_dst_file)
          else:
            shutil.copy(final_src_file, final_dst_file)
        print('Copied '+final_src_file+' to '+final_dst_file)
    print('\n')




class DeployDirectory:
  def __init__(self):
    self.vulkan_sdk_path = os.environ["VULKAN_SDK"]
    self.glsl_compiler_path = self.vulkan_sdk_path + '/Bin/glslc.exe'
    self.current_dir = os.getcwd()
    self.project_root_dir = os.getcwd() + '/..'
    self.shader_dir = self.project_root_dir + '/src/shaders'
    self.texture_dir = self.project_root_dir + '/src/textures'
    self.includes_dir = self.project_root_dir + '/src'
    self.target_dir = self.project_root_dir + '/../thirdparty/rendersystem'


class Deployer:
  def __init__(self):
    self.deploy_dirs = DeployDirectory()

  def compileShaders(self):
    shaderCompiler = ShaderCompiler(self.deploy_dirs.shader_dir, self.deploy_dirs.glsl_compiler_path)
    shaderCompiler.compile()

  def deployAssets(self):
    #deploy shaders and textures
    src_dirs = [self.deploy_dirs.shader_dir+'/spv', self.deploy_dirs.texture_dir]
    dst_sub_dirs = ['shaders', 'textures']
    copier = ArtifactCopier(src_dirs, self.deploy_dirs.target_dir+'/assets', dst_sub_dirs)
    
    # #deploy header files
    # src_dirs = [self.deploy_dirs.project_root_dir+'/src']
    # dst_sub_dirs = ['includes']
    # copier = ArtifactCopier(src_dirs, self.deploy_dirs.target_dir, dst_sub_dirs)
    
    #deploy .lib, .dll and .pdb
    src_dll_file = self.deploy_dirs.project_root_dir+'/i386/x64/Debug/VkRenderSystem.dll'
    dst_dll_file = self.deploy_dirs.target_dir+'/bin/Debug64/VkRenderSystem.dll'
    shutil.copy(src_dll_file, dst_dll_file)
    print('Copied '+src_dll_file+' to '+dst_dll_file)
    
    src_lib_file = self.deploy_dirs.project_root_dir+'/i386/x64/Debug/VkRenderSystem.lib'
    dst_lib_file = self.deploy_dirs.target_dir+'/bin/Debug64/VkRenderSystem.lib'
    shutil.copy(src_lib_file, dst_lib_file)
    print('Copied '+src_lib_file+' to '+dst_lib_file)
    
    src_pdb_file = self.deploy_dirs.project_root_dir+'/i386/x64/Debug/VkRenderSystem.pdb'
    dst_pdb_file = self.deploy_dirs.target_dir+'/bin/Debug64/VkRenderSystem.pdb'
    shutil.copy(src_pdb_file, dst_pdb_file)
    print('Copied '+src_pdb_file+' to '+dst_pdb_file)
    
    
  def printData(self):
    print('\nPost build directories..')
    print('Found Vulkan SDK path: ', self.deploy_dirs.vulkan_sdk_path)
    print('Found current directory: ', self.deploy_dirs.current_dir)
    print('Found project root directory: ', self.deploy_dirs.project_root_dir)
    print('Found shader directory: ', self.deploy_dirs.shader_dir)
    print('Found texture directory: ', self.deploy_dirs.texture_dir)
    

driver = Deployer()
driver.printData()
driver.compileShaders()
driver.deployAssets()
 
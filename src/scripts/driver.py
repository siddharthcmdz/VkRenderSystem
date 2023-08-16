
import os
import subprocess
import shutil

debug_mode = False

class ShaderCompiler:
  def __init__(self, shader_dir, glslcompiler_path):
    self.shader_dir = shader_dir
    self.spv_dir = self.shader_dir + '/spv/'
    self.glsl_compiler_path = glslcompiler_path
    
  def build(self):
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
        if debug_mode:
          print(self.glsl_compiler_path + ' ' + srcfile + ' -o ' + dstfile)
        else:
          subprocess.call([self.glsl_compiler_path, srcfile, '-o', dstfile] )
          print('Completed compiling', file)
    print('\n')
        


class ResourceCopier:
  def __init__(self, current_dir, resource_dirs, target_dir, dst_sub_dir):
    print('Copying resources to build folder...')
    #make the folder if it doesnt exist
    for sub_folder in dst_sub_dir:
      final_dst_dir = target_dir + '/'+sub_folder
      if not os.path.exists(final_dst_dir):
        os.makedirs(final_dst_dir)
    
    # copy the folders
    for idx, src_folder in enumerate(resource_dirs):
      src_files = os.listdir(src_folder)
      for src_file in src_files:
        final_dst_file = target_dir+'/'+dst_sub_dir[idx]+'/'+src_file
        final_src_file = src_folder + '/' + src_file
        if debug_mode:
          print('copy '+final_src_file+' to '+final_dst_file)
        else:
          shutil.copy(final_src_file, final_dst_file)
          print('copied '+final_src_file+' to '+final_dst_file)
    print('\n')


class BuildDriver:
  def __init__(self):
    self.vulkan_sdk_path = os.environ["VULKAN_SDK"]
    self.glsl_compiler_path = self.vulkan_sdk_path + '/Bin/glslc.exe'
    self.current_dir = os.getcwd()
    self.shader_dir = self.current_dir + '/src/shaders'
    self.texture_dir = self.current_dir + '/src/textures'
    self.target_dir = self.current_dir + '/x64/Debug'

  def compileShaders(self):
    shaderCompiler = ShaderCompiler(self.shader_dir, self.glsl_compiler_path)
    shaderCompiler.build()

  def copyResources(self):
    src_dirs = [self.shader_dir+'/spv', self.texture_dir]
    dst_sub_dirs = ['shaders', 'textures']
    copier = ResourceCopier(self.current_dir, src_dirs, self.target_dir, dst_sub_dirs)
    
  def printData(self):
    print('\nPreparing build directory..')
    print('Found Vulkan SDK path: ', self.vulkan_sdk_path)
    print('Found current directory: ', self.current_dir)
    print('Found shader directory: ', self.shader_dir)
    print('Found texture directory: ', self.texture_dir)
    
    
driver = BuildDriver()
#driver.printData()
driver.compileShaders()
driver.copyResources()

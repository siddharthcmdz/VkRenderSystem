
import os
import subprocess

class ShaderCompiler:
  def __init__(self, shader_dir, glslcompiler_path):
    self.shader_dir = shader_dir
    self.spv_dir = self.shader_dir + '/spv/'
    self.glsl_compiler_path = glslcompiler_path
    
  def build(self):
    shader_files = os.listdir(self.shader_dir)
    for file in shader_files:
      if file.find('.vert') or file.find('.frag'):
        postfix = 'Vert'
        if '.frag' in file:
          postfix = 'Frag'
      
        dstfile = self.spv_dir+file+postfix+'.spv'
        print(self.glsl_compiler_path + ' ' + file + ' -o ' + self.spv_dir+file+postfix+'.spv')
        # subprocess.call([self.glsl_compiler_path, file, '-o', self.spv_dir+file+postfix+'.spv'] )
      
    print(shader_files)

class BuildDriver:
  
  def __init__(self):
    self.vulkan_sdk_path = os.environ["VULKAN_SDK"]
    self.glsl_compiler_path = self.vulkan_sdk_path + '/Bin/glslc.exe'
    self.current_dir = os.getcwd()
    self.shader_dir = self.current_dir + '/../shaders'
    self.texture_dir = self.current_dir + "/../textures"
    
  def build(self):
    shaderCompiler = ShaderCompiler(self.shader_dir, self.glsl_compiler_path)
    shaderCompiler.build()
    
  def printData(self):
    print('\nDirectories')
    print('-----------')
    print('Vulkan SDK path: ', self.vulkan_sdk_path)
    print('Current directory: ', self.current_dir)
    print('Shader directory: ', self.shader_dir)
    print('Texture directory: ', self.texture_dir)
    
    
driver = BuildDriver()
driver.printData()
driver.build()


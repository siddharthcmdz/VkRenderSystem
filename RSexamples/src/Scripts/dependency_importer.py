
import os, shutil

print_mode = False

class ArtifactCopier:
  def __init__(self, src_dirs, target_dir, dst_sub_dir='', ext=''):
    print('Copying resources to build folder...')
    #make the folder if it doesnt exist
    if len(dst_sub_dir) != 0:
      for sub_folder in dst_sub_dir:
        final_dst_dir = target_dir + '/'+sub_folder
        if not os.path.exists(final_dst_dir):
          os.makedirs(final_dst_dir)
    
    # copy the folders
    for idx, src_folder in enumerate(src_dirs):
      print('src_folder: ', src_folder)
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

class IngestDirectory:
  def __init__(self):
    self.current_dir = os.getcwd()
    self.project_root_dir = os.getcwd() + '/..'
    self.rs_thirdparty_dir = self.project_root_dir + '/../thirdparty/rendersystem'
    self.rs_assets_dir = self.project_root_dir + '/../thirdparty/rendersystem/assets'
    self.rs_assets_texture_dir = self.rs_assets_dir+'/textures'
    self.rs_assets_shader_dir = self.rs_assets_dir+'/shaders'
    self.rs_includes_dir = self.rs_thirdparty_dir + '/includes'
    self.rs_bin_debug64_folder = self.rs_thirdparty_dir + '/bin/Debug64'
    
    self.target_dir = self.project_root_dir + '/i386/x64/Debug'


class Ingestor:
  def __init__(self):
    self.ingest_directory = IngestDirectory()
  
  def ingestRSbinaries(self):
    rs_debug64_dll_src = self.ingest_directory.rs_bin_debug64_folder+'/VkRenderSystem.dll'
    rs_debug64_lib_src = self.ingest_directory.rs_bin_debug64_folder+'/VkRenderSystem.lib'
    
    rs_debug64_dll_dst = self.ingest_directory.target_dir+'/VkRenderSystem.dll'
    rs_debug64_lib_dst = self.ingest_directory.target_dir+'/VkRenderSystem.lib'
    
    shutil.copy(rs_debug64_dll_src, rs_debug64_dll_dst)
    shutil.copy(rs_debug64_lib_src, rs_debug64_lib_dst)
    
  def ingestAssets(self):
    #ingest shaders
    shader_copier = ArtifactCopier([self.ingest_directory.rs_assets_shader_dir], self.ingest_directory.target_dir, ['shaders'])
    
    #ingest textures
    texture_copier = ArtifactCopier([self.ingest_directory.rs_assets_texture_dir], self.ingest_directory.target_dir, ['textures'])


ingestor = Ingestor()
ingestor.ingestRSbinaries()
ingestor.ingestAssets()

      
  
    
    
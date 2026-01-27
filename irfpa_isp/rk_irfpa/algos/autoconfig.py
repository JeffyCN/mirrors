#!/usr/bin/python3
# -*- coding: UTF-8 -*-

import sys
import os

def get_val_in_commen(de, idx):
    de = de.replace('[', '')
    de = de.replace(']', '')
    l = de.split(",")
    idx = min(idx, len(l) -1)
    return l[idx]

class RkIrfpaParamsVar:
    is_basic = True
    is_array = 0
    desc = None
    name = ''
    typename = ''
    vartype = ''
    num = ''
    default = None
    num_real = 0

    def gen_init_statement(self):
        t = ""
        if self.vartype == "struct" or self.vartype == "struct_list":
            if self.is_array == 0:
                t = "    %s_init(&p->%s);\n" %(self.typename, self.name)
                return t
            else:
                for i in range(self.num_real):
                    t = t + "    %s_init(&p->%s[%d]);\n" %(self.typename, self.name, i)
                return t

        if self.desc:
            self.default = self.desc["default"]
        if self.default == None:
            t = "    // %s No default value\n" %(self.name)
            return t
            
        if self.is_array == 0:
            if (self.typename == "string"):
                t = '    strcpy(p->%s, "%s");\n' %(self.name, self.default)
            else:
                t = "    p->%s = %s;\n" %(self.name, self.default)
            return t
        else:
            # 处理字符串数组的特殊情况
            if self.typename == "string":
                t = '    strcpy(p->%s, %s);\n' %(self.name, self.default)
                return t
            else:
                for i in range(self.num_real):
                    tmp = get_val_in_commen(self.default, i)
                    t = t + "    p->%s[%d] = %s;\n" %(self.name, i, tmp)
                return t

    def gen_j2s_statement(self):
        t = '    obj = cJSON_GetObjectItemCaseSensitive(root, "%s");\n' %(self.name)
        if self.vartype == "struct" or self.vartype == "struct_list":
            if self.is_array == 0:
                t = t + '    %s_from_json(&p->%s, obj);\n'  %(self.typename, self.name)
            else:
                t = t + '   {\n'
                t = t + '       int i = 0;\n'
                t = t + '       cJSON_ArrayForEach(j, obj) {\n'
                t = t + '           if (i < %d) {\n' %(self.num_real)
                t = t + '               %s_from_json(&p->%s[i], j);\n' %(self.typename, self.name)
                t = t + '               i++;\n'
                t = t + '           }\n'
                t = t + '       }\n'
                t = t + '   }\n'
        elif self.vartype == "enum":
            if self.is_array == 0:
                t = t + '    if (obj && cJSON_IsString(obj)) {\n'
                t = t + '        p->%s = (%s)enum_string_to_value("%s", obj->valuestring, (int)p->%s);\n' %(self.name, self.typename, self.typename, self.name)
                t = t + '    } else if (obj && cJSON_IsNumber(obj)) {\n'
                t = t + '        p->%s = (%s)obj->valueint;\n' %(self.name, self.typename)
                t = t + '    }\n'
            else:
                t = t + '   {\n'
                t = t + '       int i = 0;\n'
                t = t + '       cJSON_ArrayForEach(j, obj) {\n'
                t = t + '           if (i < %d) {\n' %(self.num_real)
                t = t + '               if (cJSON_IsString(j)) {\n'
                t = t + '                   p->%s[i] = (%s)enum_string_to_value("%s", j->valuestring, (int)p->%s[i]);\n' %(self.name, self.typename, self.typename, self.name)
                t = t + '               } else if (cJSON_IsNumber(j)) {\n'
                t = t + '                   p->%s[i] = (%s)j->valueint;\n' %(self.name, self.typename)
                t = t + '               }\n'
                t = t + '               i++;\n'
                t = t + '           }\n'
                t = t + '       }\n'
                t = t + '   }\n'
        else:
            if self.is_array == 0:
                if self.typename == 'string':
                    t = t + '    strcpy(p->%s, obj->valuestring);\n'  %(self.name)
                elif self.typename == 'float' or self.typename == 'double':
                    t = t + '    p->%s = (%s)obj->valuedouble;\n'  %(self.name, self.typename)
                else:
                    t = t + '    p->%s = (%s)obj->valueint;\n'  %(self.name, self.typename)
            else:
                # 处理字符串数组的特殊情况
                if self.typename == 'string':
                    t = t + '    strcpy(p->%s, obj->valuestring);\n'  %(self.name)
                else:
                    t = t + '   {\n'
                    t = t + '       int i = 0;\n'
                    t = t + '       cJSON_ArrayForEach(j, obj) {\n'
                    t = t + '           p->%s[i++] = (%s)j->valuedouble;\n' %(self.name, self.typename)
                    t = t + '       }\n'
                    t = t + '   }\n'
        return t

    def gen_s2j_statement(self):
        t = ""
        if self.vartype == "struct" or self.vartype == "struct_list":
            if self.is_array == 0:
                t = '    cJSON_AddItemToObject(obj, "%s", %s_to_json(&p->%s));\n' %(self.name, self.typename, self.name)
                return t
            else:
                t = '    {\n'
                t = t + '        cJSON *arr = cJSON_CreateArray();\n'
                t = t + '        for (int i=0; i<%d; i++) {\n' %(self.num_real)
                t = t + '            cJSON_AddItemToArray(arr, %s_to_json(&p->%s[i]));\n' %(self.typename, self.name)
                t = t + '        }\n'
                t = t + '        cJSON_AddItemToObject(obj, "%s", arr);\n' %(self.name)
                t = t + '    }\n'
                return t
        elif self.vartype == "enum":
            if self.is_array == 0:
                t = '    cJSON_AddStringToObject(obj, "%s", enum_value_to_string("%s", p->%s));\n' %(self.name, self.typename, self.name)
            else:
                t = '    {\n'
                t = t + '        cJSON *arr = cJSON_CreateArray();\n'
                t = t + '        for (int i=0; i<%d; i++) {\n' %(self.num_real)
                t = t + '            cJSON_AddItemToArray(arr, cJSON_CreateString(enum_value_to_string("%s", p->%s[i])));\n' %(self.typename, self.name)
                t = t + '        }\n'
                t = t + '        cJSON_AddItemToObject(obj, "%s", arr);\n' %(self.name)
                t = t + '    }\n'
            return t

        if self.is_array == 0:
            if (self.typename == 'bool'):
                t = '    cJSON_AddBoolToObject(obj, "%s", p->%s);\n' %(self.name, self.name)
            elif (self.typename == 'string'):
                t = '    cJSON_AddStringToObject(obj, "%s", p->%s);\n' %(self.name, self.name)
            else:
                t = '    cJSON_AddNumberToObject(obj, "%s", p->%s);\n' %(self.name, self.name)
            return t
        else:
            # 处理字符串数组的特殊情况
            if self.typename == 'string':
                t = '    cJSON_AddStringToObject(obj, "%s", p->%s);\n' %(self.name, self.name)
                return t
            else:
                t = '    {\n'
                t = t + '        cJSON *arr = cJSON_CreateArray();\n'
                t = t + '        for (int i=0; i<%d; i++) {\n' %(self.num_real)
                if self.typename == 'bool':
                    t = t + '            cJSON_AddItemToArray(arr, cJSON_CreateBool(p->%s[i]));\n' %(self.name)
                else:
                    t = t + '            cJSON_AddItemToArray(arr, cJSON_CreateNumber(p->%s[i]));\n' %(self.name)
                t = t + '        }\n'
                t = t + '        cJSON_AddItemToObject(obj, "%s", arr);\n' %(self.name)
                t = t + '    }\n'
                return t

class RkIrfpaParamsStructDef:
    name = ''
    varlist = []

    def gen_init_function_protype(self):
        return "void %s_init(%s *p);\n"  %(self.name, self.name)
    
    def gen_init_functions(self):
        t = "void %s_init(%s *p) {\n" %(self.name, self.name)
        for v in self.varlist:
            t = t + v.gen_init_statement()
        t = t + "}\n"
        return t

    def gen_s2j_function_protype(self):
        return "cJSON* %s_to_json(%s *p);\n"  %(self.name, self.name)
    
    def gen_s2j_functions(self):
        t = "cJSON* %s_to_json(%s *p) {\n" %(self.name, self.name)
        t = t + '    cJSON *obj = cJSON_CreateObject();\n'
        for v in self.varlist:
            t = t + v.gen_s2j_statement()
        t = t + "    return obj;\n"
        t = t + "}\n"
        return t

    def gen_j2s_function_protype(self):
        return "void %s_from_json(%s *p, cJSON *root);\n"  %(self.name, self.name)
    
    def gen_j2s_functions(self):
        t = "void %s_from_json(%s *p, cJSON *root) {\n" %(self.name, self.name)
        t = t + '    cJSON *obj = NULL; cJSON *j = NULL;\n'
        for v in self.varlist:
            t = t + v.gen_j2s_statement()
        t = t + "}\n"
        return t

class RkIrfpaParams:
    num_type = [
            "int8_t",
            "uint8_t",
            "int16_t",
            "uint16_t",
            "int32_t",
            "uint32_t",
            "float",
            ] 
     
    base_type = num_type + ["bool", "string"]
    user_define = {}
    user_type = []
    enum_types = []
    enum_values = {}  # 存储枚举类型到枚举值的映射
    typedef_struct = {}
    typedef_enum = {}
    desc = {} 
    cur_desc = [] 
    cur_reg = ""

    def handle_define(self, text):
        l = text.split()
        if len(l) == 3:
            self.user_define[l[1]] = l[2]

    def desc_get_val(self, text, name, p):
        if name != None:
            p0 = text.find(name, p)
            if p0 == -1:
                return None, p
            p1 = text.index("(", p0)
            p2 = text.index(")", p1)
            val = text[p1+1:p2].strip()
            return val, p2
        else:
            p0 = p
            p1 = text.index('"', p0)
            p2 = text.index('"', p1+ 1)
            val = text[p1+1:p2].strip()
            return val, p2+1

    def handle_generic_desc(self, text):
        pos = 0
        desc = {}
        desc["alias"], pos = self.desc_get_val(text, "M4_ALIAS", 0)
        desc["type"], pos = self.desc_get_val(text, "M4_TYPE", 0)
        desc["size_ex"], pos = self.desc_get_val(text, "M4_SIZE_EX", 0)
        desc["range_ex"], pos = self.desc_get_val(text, "M4_RANGE_EX", 0)
        desc["default"], pos = self.desc_get_val(text, "M4_DEFAULT", 0)
        desc["digit_ex"], pos = self.desc_get_val(text, "M4_DIGIT_EX", 0)
        desc["dynamic_ex"], pos = self.desc_get_val(text, "M4_DYNAMIC_EX", 0)
        desc["hide_ex"], pos = self.desc_get_val(text, "M4_HIDE_EX", 0)
        desc["ro"], pos = self.desc_get_val(text, "M4_RO", 0)
        desc["order"], pos = self.desc_get_val(text, "M4_ORDER", 0)
        desc["notes"], pos = self.desc_get_val(text, "M4_NOTES", 0)
        return desc

    def handle_comment(self, text):
        desc = {}
        if "M4_GENERIC_DESC" in text:
            desc = self.handle_generic_desc(text)
        self.cur_desc.append(desc)

    def get_val(self, text):
        v = RkIrfpaParamsVar()
        text = text.strip()
        assert text[-1] == ';'
        text = text[:-1]

        l = text.split()
        if l[0] == "int":
            l[0] = "int32_t"

        if l[0] == "char" and "[" in l[1]:
            v.vartype = "base"
            l[0] = "string"  # 将char数组标记为string类型
        elif l[0] in self.base_type:
            v.vartype = "base"
        elif l[0] in self.user_type:
            # 检查是否有struct_list类型描述
            if len(self.cur_desc) > 0:
                last_desc = self.cur_desc[-1]
                if last_desc and last_desc.get("type") == "struct_list":
                    v.vartype = "struct_list"
                else:
                    v.vartype = "struct"
            else:
                v.vartype = "struct"
        elif l[0] in self.enum_types:
            v.vartype = "enum"
        else:
            assert False, "unknown type " + l[0]

        v.typename = l[0]

        if "][" in l[1]:
            v.is_array = 2
        elif "[" in l[1]:
            v.is_array = 1
            p1 = l[1].index("[")
            p2 = l[1].index("]", p1)
            v.num = l[1][p1+1:p2].strip()
            v.name = l[1][:p1]
            if v.num in self.user_define.keys():
                v.num_real = int(self.user_define[v.num])
            elif v.num.isdigit():
                v.num_real = int(v.num)
            else:
                assert False, "%s %s not a number" % (v.name, v.num)
        else:
            v.is_array = 0
            v.name = l[1]
        return v

    def get_enum(self, text):
        v = {}
        text = text.strip()
        if '=' in text:
            l = text.split('=')
            v["name"] = l[0].strip()
            v["val"] = l[1].strip()
        else:
            v["name"] = text.strip()
            v["val"] = None
        return v

    def handle_typedef_enum(self, text):
        eu = {}
        vals = []
        p1 = text.index("{")
        p2 = text.index("}", p1)
        val_text = text[p1+1:p2].strip()
        l = val_text.split(",")
        
        # 自动分配枚举值
        current_value = 0
        for vt in l:
            if len(vt) > 0:
                v = self.get_enum(vt)
                # 如果没有赋值，自动分配值
                if v["val"] is None:
                    v["val"] = str(current_value)
                    current_value += 1
                else:
                    # 如果已经有赋值，更新当前值
                    try:
                        current_value = int(v["val"]) + 1
                    except ValueError:
                        # 如果值不是数字，保持当前值不变
                        pass
                vals.append(v)

        p1 = text.index("}")
        p2 = text.index(";", p1)
        eu["name"] = text[p1+1:p2].strip()
        eu["vals"] = vals
        self.enum_types.append(eu["name"])
        
        # 存储枚举值信息
        self.enum_values[eu["name"]] = vals
        
        return eu

    def handle_typedef_struct(self, text):
        st = RkIrfpaParamsStructDef()
        p1 = text.index("{")
        p2 = text.index("}", p1)
        val_text = text[p1+1:p2].strip()
        l = val_text.split(";")

        p1 = text.index("}")
        p2 = text.index(";", p1)
        st.name = text[p1+1:p2].strip()
        st.varlist = self.vals_in_struct
        self.user_type.append(st.name)

        return st

    def get_default(self, v, comment):
        comment = comment.strip()
        if len(comment) == 0:
            return

        if v.vartype == "base":
            if v.typename == "string":
                v.default = '"%s"' % (comment)
            else:
                v.default = comment

    def parse_header(self, file):
        hf = open(file, "r", encoding = 'utf8')
        lines = hf.readlines()
        hf.close()

        in_typedef_struct = False
        struct_text = ""
        in_typedef_enum = False
        enum_text = ""
        in_multiline_comment = False
        comment_text = ""
        in_use_less_line = False

        for i, line in enumerate(lines):
            line = line.strip()
            line_commnet = ""
            if "#if 0" in line:
                in_use_less_line = True
            if "#endif" in line:
                in_use_less_line = False
                continue
            if in_use_less_line:
                continue

            if in_multiline_comment:
                if "*/" in line:
                    in_multiline_comment = False
                    p = line.index("*/")
                    comment_text += line[:p]
                    self.handle_comment(comment_text)
                else:
                    comment_text += line
                continue
            if "/*" in line:
                if "*/" not in line:
                    in_multiline_comment = True
                    comment_text = ""
                    p = line.index("/*") + 2
                    comment_text += line[p:]
                else:
                    p1 = line.index("/*") + 2
                    p2 = line.index("*/")
                    comment_text += line[p1:p2]
                    self.handle_comment(comment_text)
                continue

            if "//" in line:
                comment_text = ""
                p = line.index("//")
                line_commnet += line[p+2:]
                line = line[:p]
                line = line.strip()

            if in_typedef_struct:
                if "}" in line:
                    in_typedef_struct = False
                    struct_text += line
                    st = self.handle_typedef_struct(struct_text)
                    self.typedef_struct[st.name] = st
                else:
                    if ";" in line:
                        v = self.get_val(line)
                        # 处理注释中的类型信息
                        if len(self.cur_desc) > 0:
                            v.desc = self.cur_desc.pop()
                            # 如果描述中指定了struct_list类型，则更新vartype
                            if v.desc and v.desc.get("type") == "struct_list":
                                v.vartype = "struct_list"
                        self.get_default(v, line_commnet)
                        self.vals_in_struct.append(v)
                    struct_text += line

            if in_typedef_enum:
                if "}" in line:
                    in_typedef_enum = False
                    enum_text += line
                    eu = self.handle_typedef_enum(enum_text)
                    self.typedef_enum[eu["name"]] = eu
                else:
                    enum_text += line

            if "typedef struct" in line:
                self.vals_in_struct = []
                struct_text = ""
                if "}" not in line:
                    in_typedef_struct = True
                    struct_text += line 
                else:
                    assert False

            if "typedef enum" in line:
                enum_text = ""
                if "}" not in line:
                    in_typedef_enum = True
                    enum_text += line 
                else:
                    assert False

            if line.startswith("#define"):
                self.handle_define(line)

    def __init__(self, file):
        self.parse_header(file)

    def gen_functions_walk_vals(self, vals, func):
        for v in vals:
            t = v.gen_init_statement()
            print(t)

    def gen_init_functions(self):
        out_text = ""
        for k, st in self.typedef_struct.items():
            out_text = out_text + st.gen_init_function_protype()
        for k, st in self.typedef_struct.items():
            out_text = out_text + st.gen_init_functions()
        return out_text

    def gen_s2j_functions(self):
        out_text = ""
        for k, st in self.typedef_struct.items():
            out_text = out_text + st.gen_s2j_function_protype()
        for k, st in self.typedef_struct.items():
            out_text = out_text + st.gen_s2j_functions()
        return out_text

    def gen_j2s_functions(self):
        out_text = ""
        for k, st in self.typedef_struct.items():
            out_text = out_text + st.gen_j2s_function_protype()
        for k, st in self.typedef_struct.items():
            out_text = out_text + st.gen_j2s_functions()
        return out_text

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Useage: %s <file>" % (sys.argv[0]))
        exit(0)

    script_path = os.path.realpath(__file__)
    script_dir = os.path.dirname(script_path)
    header_file = os.path.join(script_dir, sys.argv[1])
    outfile = os.path.splitext(header_file)[0] + ".cpp"
    print("generating %s" % (outfile));

    h = RkIrfpaParams(header_file)

    of = open(outfile, "w", encoding = 'utf8')
    of.write('#include "rk_irfpa_params_common.h"\n')
    of.write('#include "%s"\n' % (os.path.basename(header_file)))
    of.write('#include <string.h>\n')
    
    # 添加枚举辅助函数的声明
    of.write('''
// 枚举转换辅助函数
static const char* enum_value_to_string(const char* enum_type, int value);
static int enum_string_to_value(const char* enum_type, const char* str, int default_value);

''')
    
    t = h.gen_init_functions()
    of.write(t)

    t = h.gen_s2j_functions()
    of.write(t)

    t = h.gen_j2s_functions()
    of.write(t)
    
    # 添加枚举辅助函数的实现
    of.write('''
// 枚举值到字符串的转换函数
static const char* enum_value_to_string(const char* enum_type, int value) {
''')
    
    # 为每个枚举类型生成转换代码
    for enum_name, enum_vals in h.enum_values.items():
        of.write('    if (strcmp(enum_type, "%s") == 0) {\n' % enum_name)
        of.write('        switch (value) {\n')
        for val in enum_vals:
            if val["val"] is not None:
                of.write('            case %s: return "%s";\n' % (val["val"], val["name"]))
            else:
                of.write('            // 注意: %s 没有明确的值，需要手动设置\n' % val["name"])
        of.write('            default: return "UNKNOWN";\n')
        of.write('        }\n')
        of.write('    }\n')
    
    of.write('''
    return "UNSUPPORTED_ENUM_TYPE";
}

// 字符串到枚举值的转换函数
static int enum_string_to_value(const char* enum_type, const char* str, int default_value) {
''')
    
    # 为每个枚举类型生成转换代码
    for enum_name, enum_vals in h.enum_values.items():
        of.write('    if (strcmp(enum_type, "%s") == 0) {\n' % enum_name)
        for val in enum_vals:
            if val["val"] is not None:
                of.write('        if (strcmp(str, "%s") == 0) return %s;\n' % (val["name"], val["val"]))
            else:
                of.write('        // 注意: %s 没有明确的值，需要手动设置\n' % val["name"])
        of.write('        return default_value;\n')
        of.write('    }\n')
    
    of.write('''
    return default_value;
}
''')
    
    of.close()
// i18n.h
#ifndef I18N_H
#define I18N_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// 语言项最大数量
#define MAX_STRINGS 1000
// 字符串最大长度
#define MAX_STRING_LEN 256
// 路径最大长度
#define MAX_PATH_LEN 512

typedef struct {
    char* strings[MAX_STRINGS];
    char* translations[MAX_STRINGS];
    int count;
    char current_lang[32];
    char lang_path[MAX_PATH_LEN];  // 新增：语言文件路径
} I18N_Config;

// 全局配置
static I18N_Config g_i18n = {0};

// 去除字符串两端的空白字符
static char* trim(char* str) {
    char* end;
    while(isspace((unsigned char)*str)) str++;
    if(*str == 0) return str;
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

// 从INI文件加载语言
static int load_language(const char* full_path) {
    FILE* file = fopen(full_path, "r");
    if (!file) {
        fprintf(stderr, "Cannot open language file: %s\n", full_path);
        return 0;
    }
    
    // 清理之前的数据
    cleanup_i18n();
    
    char line[MAX_STRING_LEN];
    char *key, *value;
    
    while (fgets(line, sizeof(line), file)) {
        // 跳过注释和空行
        if (line[0] == ';' || line[0] == '#' || line[0] == '\n') continue;
        
        // 查找等号
        char* delimiter = strchr(line, '=');
        if (delimiter) {
            *delimiter = '\0';
            key = trim(line);
            value = trim(delimiter + 1);
            
            // 移除结尾的换行符
            char* newline = strchr(value, '\n');
            if (newline) *newline = '\0';
            
            // 存储键值对
            if (g_i18n.count < MAX_STRINGS) {
                g_i18n.strings[g_i18n.count] = strdup(key);
                g_i18n.translations[g_i18n.count] = strdup(value);
                g_i18n.count++;
            }
        }
    }
    
    fclose(file);
    return 1;
}

// 获取翻译后的字符串
static const char* _(const char* str) {
    for (int i = 0; i < g_i18n.count; i++) {
        if (strcmp(g_i18n.strings[i], str) == 0) {
            return g_i18n.translations[i];
        }
    }
    return str;  // 如果没找到翻译，返回原字符串
}

// 初始化语言系统
static int init_i18n(const char* lang_path, const char* lang) {
    // 保存语言文件路径
    strncpy(g_i18n.lang_path, lang_path, MAX_PATH_LEN - 1);
    
    // 构建完整的文件路径
    char full_path[MAX_PATH_LEN];
    snprintf(full_path, sizeof(full_path), "%s/lang_%s.ini", lang_path, lang);
    
    // 加载语言文件
    if (load_language(full_path)) {
        strncpy(g_i18n.current_lang, lang, sizeof(g_i18n.current_lang) - 1);
        return 1;
    }
    return 0;
}

// 切换语言
static int switch_language(const char* lang) {
    char full_path[MAX_PATH_LEN];
    snprintf(full_path, sizeof(full_path), "%s/lang_%s.ini", g_i18n.lang_path, lang);
    return load_language(full_path);
}

// 清理资源
static void cleanup_i18n() {
    for (int i = 0; i < g_i18n.count; i++) {
        free(g_i18n.strings[i]);
        free(g_i18n.translations[i]);
    }
    g_i18n.count = 0;
}

#endif // I18N_H
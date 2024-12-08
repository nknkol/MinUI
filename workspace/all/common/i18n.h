// i18n.h
#ifndef I18N_H
#define I18N_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "defines.h"
// 使用项目定义的路径
#define LANG_PATH RES_PATH "/lang"

// 语言项最大数量
#define MAX_STRINGS 1000
// 字符串最大长度
#define MAX_STRING_LEN 256

typedef struct {
    char** ids;                  // 文本ID数组
    char** translations;         // 对应的翻译文本
    int count;                   // 当前项数量
    char current_lang[32];       // 当前语言
} I18N_Config;

// 全局配置
static I18N_Config g_i18n = {0};

// 从INI文件加载语言
static int load_language(const char* lang) {
    char filepath[512];
    snprintf(filepath, sizeof(filepath), LANG_PATH "/lang_%s.ini", lang);
    
    FILE* file = fopen(filepath, "r");
    if (!file) {
        printf("Cannot open language file: %s\n", filepath);
        return 0;
    }
    
    // 首次加载时初始化数组
    if (!g_i18n.ids) {
        g_i18n.ids = (char**)malloc(MAX_STRINGS * sizeof(char*));
        g_i18n.translations = (char**)malloc(MAX_STRINGS * sizeof(char*));
    } else {
        // 清理之前的数据
        for (int i = 0; i < g_i18n.count; i++) {
            free(g_i18n.translations[i]);
        }
    }
    g_i18n.count = 0;
    
    char line[MAX_STRING_LEN];
    char *key, *value;
    
    while (fgets(line, sizeof(line), file)) {
        // 跳过注释和空行
        if (line[0] == ';' || line[0] == '#' || line[0] == '\n') continue;
        
        // 查找等号
        char* delimiter = strchr(line, '=');
        if (delimiter) {
            *delimiter = '\0';
            key = line;
            value = delimiter + 1;
            
            // 移除前后空格
            while (*key == ' ' || *key == '\t') key++;
            while (*value == ' ' || *value == '\t') value++;
            
            char* end = value + strlen(value) - 1;
            while (end > value && (*end == '\n' || *end == '\r' || *end == ' ' || *end == '\t')) end--;
            *(end + 1) = '\0';
            
            end = key + strlen(key) - 1;
            while (end > key && (*end == ' ' || *end == '\t')) end--;
            *(end + 1) = '\0';
            
            // 存储ID和翻译
            if (g_i18n.count < MAX_STRINGS) {
                if (g_i18n.count == 0) {
                    // 第一次加载时保存ID
                    g_i18n.ids[g_i18n.count] = strdup(key);
                }
                g_i18n.translations[g_i18n.count] = strdup(value);
                g_i18n.count++;
            }
        }
    }
    
    fclose(file);
    strncpy(g_i18n.current_lang, lang, sizeof(g_i18n.current_lang) - 1);
    return 1;
}

// 获取翻译文本
static const char* get_text(const char* text_id) {
    for (int i = 0; i < g_i18n.count; i++) {
        if (strcmp(g_i18n.ids[i], text_id) == 0) {
            return g_i18n.translations[i];
        }
    }
    return text_id;  // 找不到翻译时返回ID本身
}

// 初始化语言系统
static int init_i18n(const char* lang) {
    return load_language(lang);
}

// 切换语言
static int switch_language(const char* lang) {
    return load_language(lang);
}

// 清理资源
static void cleanup_i18n() {
    if (g_i18n.ids) {
        for (int i = 0; i < g_i18n.count; i++) {
            if (i == 0) {  // 只在第一个语言文件中存储了ID
                free(g_i18n.ids[i]);
            }
            free(g_i18n.translations[i]);
        }
        free(g_i18n.ids);
        free(g_i18n.translations);
        g_i18n.ids = NULL;
        g_i18n.translations = NULL;
    }
    g_i18n.count = 0;
}

#endif // I18N_H
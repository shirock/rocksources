#!/usr/bin/python3
import os
import sys

# 試驗中! 是否合併二級簡碼。
MIX_LV2_CODE = True

table = {}
lv2_table = {}
spcn_list = ['①', '②', '③', '④', '⑤', '⑥', '⑦', '⑧', '⑨', '⑩']

keycode_sorted = [
    'q', 'a', 'z', 
    'w', 's', 'x', 
    'e', 'd', 'c', 
    'r', 'f', 'v', 
    't', 'g', 'b',
    'y', 'h', 'n',
    'u', 'j', 'm',
    'i', 'k', ',',
    'o', 'l', '.',
    'p', ';', '/']

# 行列簡碼表
with open('ar30_hw.txt', 'r', encoding='utf8') as fh:
    while True:
        line = fh.readline()
        if not line:
            break
        if line.startswith('#'):
            continue

        key = line[0:2].strip().lower()
        words = line[2:].strip().split(' ')

        if len(key) > 1:
            lv2_table[key] = words # 二級簡碼優先序低於特別碼，最後處理
        else:
            # 一級簡碼
            table[key] = words

# 一級簡碼的 '隨' 和 特殊碼 '的' 的位置重複了。取的捨隨。
table['t'][0] = '的'

if MIX_LV2_CODE:
    print('按字根順序建立字根表。建立中...')
    for key in lv2_table:
        table[key] = []
        for code3 in keycode_sorted:
            key3 = key + code3
            table[key3] = []
            for code4 in keycode_sorted:
                key4 = key3 + code4
                table[key4] = []
                #print(key4)

ofh = open('array30plus.txt', 'w', encoding='utf8')

with open('array30.txt', 'r', encoding='utf8') as fh:
    ofh.write(fh.readline())
    ofh.write(fh.readline())
    ofh.write(fh.readline())
    while True:
        line = fh.readline()
        if not line:
            break
        
        key, word = line.strip().split()
        #print(key, word)

        if table.get(key):
            if word not in table[key]:
                table[key].append(word)
        else:
            table[key] = [word]


if not MIX_LV2_CODE:
    # 作為踢出一級簡碼位置的補償，我把'隨'加到它首二碼的候選字清單
    table['tz'].append('隨')
else:
    # 合併二級簡碼的話，隨字改成第6位置。
    lv2_table['tz'][5] = '隨'

if MIX_LV2_CODE:
    print('正在合併二級簡碼...')
    key3s = []
    key4s = []
    for key in table:
        if len(key) == 3:
            key3s.append(key)
        elif len(key) == 4:
            key4s.append(key)

    for key in lv2_table:
        # if len(table[key]) > 1:
        #     print('需檢查是否錯誤', key, table[key])

        for word in table[key]:
            #print('check ', key, word, lv2_table[key])
            try:
                i = lv2_table[key].index(word)
            except ValueError:
                i = -1
            if i >= 0:
                lv2_table[key][i] = '□'
                #print(key, word, 'dup', lv2_table[key])

    key2_table = {} # 將二級簡碼和前二碼相符的字，都放入前二碼候選字清單
    for key in lv2_table:
        key2_table[key] = []
        for word in lv2_table[key]:
            if word != '□':
                key2_table[key].append(word)
        for key3 in key3s:
            if key3.startswith(key):
                key2_table[key] += table[key3]
            if (len(key2_table[key]) >= 10):
                break
        for key4 in key4s:
            if key4.startswith(key):
                key2_table[key] += table[key4]
            if (len(key2_table[key]) >= 10):
                break
        # print(key2_table[key])

    for key2 in lv2_table:
        for i in range(10):
            if i >= len(table[key2]):
                table[key2].append(lv2_table[key2][i])

        for i in range(10):
            if table[key2][i] != '□':
                continue

            word = False
            while len(key2_table[key2]) > 0:
                word = key2_table[key2].pop(0)
                if word not in table[key2]:
                    break
                else:
                    word =False

            if word:
                table[key2][i] = word
            else:
                table[key2][i] = spcn_list[i]

        while True:
            i = len(table[key2]) - 1
            if table[key2][i] == spcn_list[i]:
                table[key2].pop()
            else:
                break

# check
# print(table['t'])
# print(table['/'])
# print(table['z'])
# print(table[',,'])
# print('tv: ', table['tv'])
# print('tw: ', table['tw'])
# print(table['ya'])
# print(table['zg'])
# print(table['zy'])
# print(table['zz'])
# print(table['w'])
# print(table[',a'])
# print(table['zri'])

check_keys = [
    './','.a','.f','.x','/e',
    ';g',';p','c.','ca','cj',
    'cr','f,','gj','i/','jt',
    'ko','n,','no','ol','rb',
    'ri','rl','vg','vi','wj',
    'wx','xt','ya','yi','yl',
    'zg','tz','uj','z,'
]

for key in check_keys:
    print(key, lv2_table[key])
    print(key, table[key])

for key in table:
    if len(table[key]) < 1:
        continue
    for word in table[key]:
        ofh.write('{0}\t{1}\n'.format(key, word))

ofh.close()

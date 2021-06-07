#!/usr/bin/python3
import os
import sys

table = {}
# fix_table = {}

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
            #fix_table[key] = []
            break # 放棄二級簡碼
        table[key] = words

# 一級簡碼的 '隨' 和 特殊碼 '的' 的位置重複了。取的捨隨。
table['t'][0] = '的'

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
            # 2級簡碼重複字，
            # if len(key) == 2 and word in table[key]:
            #     i = table[key].index(word)
            #     table[key][i] = '□'

            # add = True
            # for i in range(len(table[key])):
            #     if table[key][i] == word or table[key][i] == '□':
            #         table[key][i] = word
            #         add = False
            #         break
            # if add:
            #     table[key].append(word)
            if word not in table[key]:
                table[key].append(word)
        else:
            table[key] = [word]

        # if len(key) > 1:
        #     skey = key[:2]
        #     if len(fix_table[skey]) < 10 and word not in fix_table[skey]:
        #         fix_table[skey].append(word)

# 作為踢出一級簡碼位置的補償，我把'隨'加到它首二碼的候選字清單
table['tz'].append('隨')

# print(fix_table)
# 以首二字根相同的字，填充二級簡碼中留空的位置。
# for key in fix_table:
#     for word in fix_table[key]:
#         if word not in table[key]:
#             for i in range(len(table[key])):
#                 if table[key][i] == '□':
#                     table[key][i] = word
#                     break

# check
print(table['t'])
print(table['/'])
print(table['z'])
print(table[',,'])
print(table['ya'])
print(table['zg'])
# print(table['zy'])
# print(table['zz'])
# print(table['w'])
# print(table[',a'])
# print(table['zri'])

for key in table:
    for word in table[key]:
        ofh.write('{0}\t{1}\n'.format(key, word))

ofh.close()

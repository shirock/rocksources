build: zhuyin-tbl.js pinyin-tbl.js quick-tbl.js array30-tbl.js

zhuyin-tbl.js: zhuyin_gen.php zhuyin.txt
	php zhuyin_gen.php

pinyin-tbl.js: pinyin_gen.php pinyin_table.txt
	php pinyin_gen.php

quick-tbl.js: scim-table_gen.php quick-classic.txt
	php scim-table_gen.php

array30-tbl.js: array30/ar30_gen.php array30/ar30.cin array30/ar30_hw.txt
	cd array30; php ar30_gen.php
	mv -f array30/array30-tbl.js .

dist: 
	./make-package --dest=. debian

clean:
	rm -rf dist
	rm zhuyin-tbl.js pinyin-tbl.js quick-tbl.js array30-tbl.js


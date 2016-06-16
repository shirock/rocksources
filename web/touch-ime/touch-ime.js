/*
Touch 輸入法 Copyright (C) 2013 遊手好閒的石頭成 <shirock.tw@gmail.com>

Touch IME is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see
<http://www.gnu.org/licenses/lgpl-3.0-standalone.html>.

You should see https://rocksources.googlecode.com/ to get more
information about Touch IME.
*/
// $Rev: 121 $
var TouchInputMethod = new (function(){

var default_input_method_engine_name = 'zhuyin';
var current_input_method_engine_name = default_input_method_engine_name;

var engines_cycle = {
    "zhuyin": "en",
    "en": "email",
    "email": "pinyin",
    "pinyin": "quick",
    "quick": "array30",
    "array30": "zhuyin"
};

var engine_info = {
    "zhuyin": {"name": "注音"},
    "en": {"name": "英數"},
    "enShift": {"name": "英數大"},
    "email": {"name": "電郵"},
    "pinyin": {"name": "拼音"},
    "array30": {"name": "行列30"},
    "cangjie3": {"name": "倉頡"},
    "cangjie5": {"name": "倉頡五"},
    "quick": {"name": "速成"}
};

var special_input_keyboard_map = {
    'password': 'en',
    'datetime': 'en',
    'date': 'en',
    'month': 'en',
    'week': 'en',
    'time': 'en',
    'localdatetime': 'en',
    'number': 'en',
    'range': 'en',
    'email': 'email'
};

var output = false; // dom cache
var candidate = false; // dom cache
var show_input_keys = false; // dom cache
var keyboard = false; // dom cache
var switch_engine_ctrls = false; // dom cache
var select_engine_ctrls = false; // dom cache

var word_table = false; // 字根表
var key_sym_map = {}; // 鍵符對照表
var keyboard_layout = []; // 鍵盤排列
var max_input_keys = 4;

var compositing = false;
var capital_mode = false;
var auto_clear_input_keys = false;
var input_keys = [];

var resources_path = ''; // see set_resources_path().

var table_xhr = new XMLHttpRequest();
if (table_xhr.overrideMimeType)
    table_xhr.overrideMimeType('text/plain');
table_xhr.addEventListener('load', function(){
    if (table_xhr.status >= 300)
        return false;
    data = table_xhr.responseText;
    update_candidates(data);
    return true;
});

var engine_xhr = new XMLHttpRequest();
if (engine_xhr.overrideMimeType)
    engine_xhr.overrideMimeType('application/json');


var keyboard_id = 'input_method_keyboard';
var candidate_id = 'input_method_candidate';
var show_input_keys_id = 'input_method_show_input_keys';

var control_classes = {
    'inputkey': 'input_method_inputkey',
    'candidates': 'input_method_candidates',
    'back_input_key': 'input_method_back_input_key',
    'clear_input_keys': 'input_method_clear_input_keys',
    'auto_clear_input_keys_mode': 'input_method_auto_clear_input_keys_mode',
    'auto_clear_input_keys_on': 'input_method_auto_clear_input_keys_on',
    'capital_toggle': 'input_method_capital_toggle',
    'capital_toggle_on': 'input_method_capital_toggle_on',
    'backspace_output_texts': 'input_method_backspace_output_texts',
    'clear_output_texts': 'input_method_clear_output_texts',
    'select_output_texts': 'input_method_select_output_texts',
    'add_space_output_texts': 'input_method_add_space_output_texts',
    'add_newline_output_texts': 'input_method_add_newline_output_texts',
    'switch_engine': 'input_method_switch_engine',
    'select_engine': 'input_method_select_engine',
    'goto_next': 'input_method_goto_next',
    'goto_previous': 'input_method_goto_previous',
    'end_composition': 'input_method_end_composition'
};

var classList_supporting = true;

/**
required id:
* keyboard_id: default is 'input_method_keyboard'.
* candidate_id: default is 'input_method_candidate'.
* show_input_keys_id: default is 'input_method_show_input_keys'.

optional classes:
* inputkey: input_method_inputkey
  字根鍵盤外觀。
* candidates: input_method_candidates
  候選字外觀。
* back_input_key: input_method_back_input_key
  指定「退一字根」控制項與外觀。
* clear_input_keys: input_method_clear_input_keys
  指定「清除字根」控制項與外觀。
* auto_clear_input_keys_mode: input_method_auto_clear_input_keys_mode
  指定「自動清除字根」控制項與外觀。
* auto_clear_input_keys_on: input_method_auto_clear_input_keys_on
  自動清除功能鈕啟動時外觀。
  若為函數而不是 CSS 類別名稱，則回呼之 (函數之 this 為控制項)，並傳入 mode 參數
  表示 auto_clear_input_keys 改變後的狀態(型態為 boolean)。
* capital_toggle: input_method_capital_toggle
  指定「Caps」(大寫鎖定)控制項與外觀。
* capital_toggle_on: input_method_capital_toggle_on
  Caps 啟動時外觀。
  若為函數而不是 CSS 類別名稱，則回呼之 (函數之 this 為控制項)，並傳入 mode 參數
  表示大寫鎖定器改變後的狀態(型態為 boolean)。
* backspace_output_texts: input_method_backspace_output_texts
  指定「回刪一字」控制項與外觀。
* clear_output_texts: input_method_clear_output_texts
  指定「清除輸入內容」控制項與外觀。
* select_output_texts: input_method_select_output_texts
  指定「選取輸入內容」控制項與外觀。
* add_space_output_texts: input_method_add_space_output_texts
  指定「空白」控制項與外觀。
* add_newline_output_texts: input_method_add_newline_output_texts
  指定「換行」控制項與外觀。
* switch_engine: input_method_switch_engine
  指定「切換輸入法」控制項與外觀。
* goto_next: input_method_goto_next
  指定「移到下一欄位」控制項與外觀。
* goto_previous: input_method_goto_previous
  指定「移到上一欄位」控制項與外觀。
* end_composition: input_method_end_composition
  指定「結束輸入法」控制項與外觀。
 */
this.set_controls = function(custom_keyboard_id, custom_candidate_id,
    custom_show_input_keys_id, optional_classes)
{
    if (custom_keyboard_id)
        keyboard_id = custom_keyboard_id;
    if (custom_candidate_id)
        candidate_id = custom_candidate_id;
    if (custom_show_input_keys_id)
        show_input_keys_id = custom_show_input_keys_id;

    for (var p in optional_classes) {
        if (p in control_classes)
            control_classes[p] = optional_classes[p];
    }
}

/**
get css selector (id and class) of controls.
 */
this.get_controls = function()
{
    return {
        'keyboard_id': keyboard_id,
        'candidate_id': candidate_id,
        'show_input_keys_id': show_input_keys_id,
        'control_classes': control_classes
    };
}

/**
 where to send converted result.
 target must be a kind of text input control.
 */
this.set_target = function(dom)
{
    output = dom;
}

/**
 which control is the target of touch-ime.
 Notice: you should not use this method to get focused control.
 */
this.get_target = function()
{
    return output;
}

/**
指定資源目錄路徑

預設是從 touch-ime.js 相同目錄下載入字根表等內容。

請注意瀏覽器的相同來源政策，如果你的字表資源放在不同來源伺服器，那麼你必須按照
W3 CORS 的建議，設置跨來源資源分享規則。
若未設置 CORS ，那麼你在此只能指定資源目錄的路徑。例如 '/js/touch-ime/' 。

See:
* http://www.w3.org/TR/cors/
* http://en.wikipedia.org/wiki/Cross-origin_resource_sharing
 */
this.set_resources_path = function(path)
{
    resources_path = path;
    if (path[path.length-1] != '/')
        resources_path += '/';
}


/**
According the class or type of input control to work with special input engine.
If engine is false, remove this rule.

1. If the control's class list contains input_class, touch-ime will auto switch
  engine to engine_code.
2. If the control's type equals to input_class, touch-ime will auto switch engine
  to engine_code.
3. Else touch-ime switch engine to default input method engine.

有些瀏覽器(特別是在移動設備上的)，會針對某些特殊型態的輸入控制項，特化它的UI。
例如碰到 datetime 的控制項，那些瀏覽器就會顯示日期挑選的 UI ，而不必使用者輸入文字。
碰到這種瀏覽器，touch-ime 就不需要理會那些控制項。
但麻煩的是目前還沒有通用方式可判斷瀏覽器有沒有為控制項提供特殊UI。
 */
this.specify_control_with_engine = function(input_class, engine_code)
{
    // see also: according_input_type_switch_engine().
    if (engine_code)
        special_input_keyboard_map[input_class] = engine_code;
    else
        delete special_input_keyboard_map[input_class];
}

/**
若你增加或刪除輸入法字表，則你必須調用此方法載入你使用的輸入法引擎資訊。
如下:
var new_engine_info = {
    "zhuyin": {
        "name": "注音"
    },
    "en": {
        "name": "英數"
    }
};

TouchInputMethod.install_engine_info(new_engine_info);

 */
this.install_engine_info = function(new_engine_info)
{
    console.log("install engine info");
    engine_info = new_engine_info;
    var code_list = [];
    for (var code in engine_info) {
        //console.log("code in engine_info:", code);
        code_list.push(code);
    }
    console.log("refresh engines_cycle");
    this.enable_engines(code_list);
}

/**
enable engines. engine_code_list is an array of engine code name.
valid code name:
* zhuyin 注音
* pinyin 拼音
* en 英數
* enShift 英數
* array30 行列30
* cangjie3 倉頡第三代
* cangjie5 倉頡第五代
* quick 倉頡速成
 */
this.enable_engines = function(engine_code_list)
{
    var next;
    if (engine_code_list.length < 1)
        return;
    current_input_method_engine_name = engine_code_list[0];
    engines_cycle = {};
    engine_code_list.forEach(function(v, i, ot){
        if (i == 0)
            default_input_method_engine_name = v;
        if (i+1 >= ot.length)
            next = 0;
        else
            next = i+1;
        engines_cycle[v] = ot[next];
    });

    refresh_select_engine_ctrls();
    this.change_engine(current_input_method_engine_name);
}

/**
change input engine to engine_code.
 */
this.change_engine = function(engine_code)
{
    changed = try_change_engine(engine_code);
    if (changed)
        default_input_method_engine_name = engine_code;
    // 每當使用者手動選擇輸入法之後，就改以該輸入法為預設。
}

/**
清除本地儲存的特殊字表

自本地儲存區(localStorage)中清除下列特殊字表的內容。
* en.tab (localStorage 名稱 input_method_en)
* s.tab (localStorage 名稱 input_method_s)
* w.tab (localStorage 名稱 input_method_w)

touch-ime 會讀取特殊字表檔的內容並保存在本地儲存區。
如果你修改了特殊字表檔的內容，則你必須調用 clean_local_table() 清除已儲存的內容，
touch-ime 才會重新讀取新的特殊字表檔內容(並再次保存於本地儲存區)。
 */
this.clean_local_table = function()
{
    if (!localStorage)
        return;
    var special_table = ['en', 's', 'w', 'ascii'];
    special_table.forEach(function(v, idx, ot) {
        console.info('remove localStorage.%s', 'input_method_'.concat(v));
        localStorage.removeItem('input_method_'.concat(v));
    });
}

// compositionstart
// compositionend
// https://developer.mozilla.org/en-US/docs/Web/API/CompositionEvent
// http://www.w3.org/TR/DOM-Level-3-Events/#event-type-compositionstart
// http://www.w3.org/TR/DOM-Level-3-Events/#event-type-compositionend

/**
TouchInputMethod 組字事件。
 */
this.oncomposition = false;

/**
TouchInputMethod 結束組字事件。
 */
this.oncompositionend = false;

/**
TouchInputMethod capital 模式改變事件。
 */
this.oncapitalization = false;

this.init = function()
{
    var elms, i;

    keyboard = document.getElementById(keyboard_id);
    candidate = document.getElementById(candidate_id);
    show_input_keys = document.getElementById(show_input_keys_id);

    if (!keyboard || !candidate) {
        console.warn("Keyboard or Candidate are not existence.");
        return;
    }

    detect_resources_path();

    // step 1
    output = document.createElement('input'); // create an unattached input control.
    // 在使用者沒有選取文字輸入控制項前，先指派一個沒掛在DOM上的文字控制項，以免發生程式錯誤。
    bind_all_text_input_focus_handler();

    classList_supporting = !!output.classList;
    //console.log("classList supporting: " + classList_supporting);

    // step 2
    toggle_auto_clear_input_keys(); // turn on.
    /*
    if (auto_clear_input_keys) {
        elms = document.getElementsByClassName(control_classes['auto_clear_input_keys_mode']);
        if (elms.length > 0) {
            for (i = 0; i < elms.length; ++i) {
                class_list_add(elms[i], control_classes['auto_clear_input_keys_on']);
            }
        }
    }
    */

    // step 3
    var input_method_toolbar_handlers = {
        "back_input_key": back_input_key,
        "clear_input_keys": clear_input_keys,
        "auto_clear_input_keys_mode": toggle_auto_clear_input_keys,
        "capital_toggle": toggle_capital,
        "backspace_output_texts": backspace_output_texts,
        "clear_output_texts": clear_output_texts,
        "select_output_texts": select_output_texts,
        "add_space_output_texts": add_space_output_texts,
        "add_newline_output_texts": add_newline_output_texts,
        "switch_engine": switch_engine,
        "goto_next": goto_next,
        "goto_previous": goto_previous,
        "end_composition": end_composition
    };

    for (var k in input_method_toolbar_handlers) {
        elms = document.getElementsByClassName(control_classes[k]);
        if (elms.length > 0) {
            for (i = 0; i < elms.length; ++i)
                elms[i].addEventListener('click', input_method_toolbar_handlers[k], false);
        }
    }
    elms = null;
    switch_engine_ctrls = document.getElementsByClassName(control_classes["switch_engine"]);

    refresh_select_engine_ctrls();

    // step 4
    load_input_method_engine();

    // step 5
    /*
    document.onkeypress = function(event) {
        //console.log("event.charCode: %s", event.charCode);
        if (event.charCode in code_map) {
            //console.log("input code: %s", code_map[event.charCode]);
            input_code(code_map[event.charCode]);
        }
        return true;
    }

    document.onkeyup = function(event) {
        if (event.keyCode == 8) {
            back_input_key();
        }
        return true;
    }
    */
//    alert('行列30輸入法網頁點取版\nCopyright (c) 2012 遊手好閒的石頭成 (shirock.tw@gmail.com)\nLicensed by GPL.');
}

function detect_resources_path() {
    // detect uri of touch-ime.
    var i, path_index;
    var loaded_scripts = document.getElementsByTagName('script');
    if (loaded_scripts.length < 1)
        return;
    for (i = 0; i < loaded_scripts.length; ++i) {
        path_index = loaded_scripts[i].src.match(/(.*\/)touch-ime.js$/);
        if (path_index)
            resources_path = path_index[1];
    }
}

function get_current_input_index() {
    var i, inputs = document.querySelectorAll('input, textarea'); //document.getElementsByTagName('input');
    for (i = 0; i < inputs.length; ++i) {
        //console.log("%s, %s", output.name, output.offsetTop);
        if (output == inputs[i])
            break;
            //console.log("equals: %s", i);
    }
    return {'inputs': inputs,
            'current': (i < inputs.length ? i : -1)};
}

function goto_previous() {
    var ic = get_current_input_index();
    var n = ic.current - 1;
    if (n < 0)
        n = ic.inputs.length - 1;
    ic.inputs[n].focus();
}

function goto_next() {
    var ic = get_current_input_index();
    var n = ic.current + 1;
    if (n >= ic.inputs.length)
        n = 0;
    ic.inputs[n].focus();

    /*
    var kev = document.createEvent('KeyboardEvent');
    kev.initKeyEvent('keypress', true, true, window,
        false,  // ctrl
        false,  // alt
        false,  // shift
        false,  // meta
        0x09,    // keyCode
        0);
    output.focus();
    console.log("dispatch event");
    //output.dispatchEvent(kev);
    //document.dispatchEvent(kev);
    // 無效。雖然dom會收到這個按鍵事件，但不會出現跳到下一欄位的效果。
    */
    // http://maythesource.com/2012/06/26/simulating-keypresses-keystrokes-with-javascript-for-use-with-greesemonkey-etc/
    // https://developer.mozilla.org/en-US/docs/Web/API/KeyboardEvent
    // https://developer.mozilla.org/en-US/docs/Web/Guide/DOM/Events/Creating_and_triggering_events
}

function refresh_select_engine_ctrls() {
    var select_ctrl, engine_code, opt;
    select_engine_ctrls = document.getElementsByClassName(control_classes["select_engine"]);
    for (i = 0; i < select_engine_ctrls.length; ++i) {
        select_ctrl = select_engine_ctrls[i];
        select_ctrl.multiple = false;
        select_ctrl.addEventListener('change', select_engine, false);
        select_ctrl.innerHTML = '';
        for (engine_code in engines_cycle) {
            opt = document.createElement('option');
            opt.value = engine_code;
            opt.textContent = engine_info[engine_code].name;
            if (engine_code == current_input_method_engine_name)
                opt.selected = true;
            select_ctrl.add(opt, null);
        }
    }
    select_ctrl = engine_code = opt = null;
}

function load_input_method_engine() {
    var tmp, tmp_tbl, tmp_keyboard;

    var word_table_file_name = current_input_method_engine_name.concat('-tbl.js');
    console.info("get table:", resources_path, word_table_file_name);
    engine_xhr.open('GET', resources_path+word_table_file_name, false); // sync call
    try {
        engine_xhr.send();
        tmp_tbl = JSON.parse(engine_xhr.responseText);
    }
    catch (e) {
        console.error("Could not load word table ", word_table_file_name);
        return false;
    }

    var keyboard_file_name = current_input_method_engine_name.concat('-keyboard.js');
    console.info("get table:", keyboard_file_name);
    engine_xhr.open('GET', resources_path+keyboard_file_name, false);
    try {
        engine_xhr.send();
        tmp_keyboard = JSON.parse(engine_xhr.responseText);
    }
    catch (e) {
        console.error("Could not load word table ", word_table_file_name);
        return false;
    }

    word_table = tmp_tbl;
    max_input_keys = tmp_keyboard.max_input_keys;
    key_sym_map = tmp_keyboard.key_sym_map;
    keyboard_layout = tmp_keyboard.keyboard_layout;
    //console.info("keyboard: %o", keyboard_layout);

    if (!keyboard) // Document is not laoded, DOMtree is not ready.
        return;

    keyboard.innerHTML = '';
    keyboard_layout.forEach(function(v, idx, ot) {
        if (v == '\n') {
            keyboard.appendChild(document.createElement('br'));
            return;
        }
        var btn = document.createElement('button');
        class_list_add(btn, control_classes['inputkey']);
        btn.value = v;
        btn.textContent = (capital_mode ? key_sym_map[v].toUpperCase() : key_sym_map[v]);
        btn.addEventListener('click', function() {
            input_code(this.value);
        }, false);
        keyboard.appendChild(btn);
    });

    // show current engine name in 'switch' button.
    for (var i = 0; i < switch_engine_ctrls.length; ++i) {
        switch_engine_ctrls[i].textContent = engine_info[current_input_method_engine_name].name;
    }

    // change current engine in 'select' control.
    for (var i = 0; i < select_engine_ctrls.length; ++i) {
        tmp = select_engine_ctrls[i];
        for (var j = 0; j < tmp.options.length; ++j) {
            if (tmp.options[j].value == current_input_method_engine_name) {
                tmp.selectedIndex = j;
                break;
            }
        }
    }

    tmp = tmp_tbl = tmp_keyboard = null;

    clear_input_keys();
    return true;
}

function try_change_engine(engine_code) {
    if (engine_code == current_input_method_engine_name)
        return true; // 相同輸入法，不必改變. 回傳 true 表示正常.
    var old_input_name = current_input_method_engine_name;
    current_input_method_engine_name = engine_code;
    var changed = load_input_method_engine();
    if (!changed) {
        current_input_method_engine_name = old_input_name;
        load_input_method_engine();
    }
    return changed;
}

function according_input_type_switch_engine() {
    // this will not change default_input_method_engine_name.
    // First check class name.
    for (var ctrl_name in special_input_keyboard_map) {
        if (class_list_contains(output, ctrl_name)) {
            try_change_engine(special_input_keyboard_map[ctrl_name]);
            return;
        }
    }

    // Second check type.
    if (output.type in special_input_keyboard_map &&
        special_input_keyboard_map[output.type] in engine_info)
    {
        try_change_engine(special_input_keyboard_map[output.type]);
        return;
    }

    try_change_engine(default_input_method_engine_name);
}

function switch_engine() {
    var backup_name = current_input_method_engine_name;
    while (true) {
        current_input_method_engine_name = engines_cycle[current_input_method_engine_name];
        if (load_input_method_engine())
            break;
        if (backup_name == current_input_method_engine_name) {
            console.error("touch-ime failed! All input engine's table files could not be loaded");
            /*這是一個特殊的情況，當此條件發生時，表示走完一輪 engines_cycle
            卻無法載入任何輸入法字表，這也意味著 touch-ime 無法運作。*/
            return;
        }
    }
    // 每當使用者手動選擇輸入法之後，就改以該輸入法為預設。
    default_input_method_engine_name = current_input_method_engine_name;
}

function select_engine() {
    var backup_name = current_input_method_engine_name;
    console.log("select engine:", this.options[this.selectedIndex].value);
    current_input_method_engine_name = this.options[this.selectedIndex].value;

    if (!load_input_method_engine()) {
        // revert
        current_input_method_engine_name = backup_name;
        for (var i = 0; i < this.options.length; ++i) {
            if (this.options[i].value == backup_name) {
                this.selectedIndex = i;
                break;
            }
        }
    }
    else {
        // 每當使用者手動選擇輸入法之後，就改以該輸入法為預設。
        default_input_method_engine_name = current_input_method_engine_name;
    }
}

function end_composition() {
    //console.log('end composition');
    if (typeof TouchInputMethod.oncompositionend == 'function')
        TouchInputMethod.oncompositionend();
    compositing = false;
}

function input_code(v) {
    if (v === undefined || v < 0 || v > 9)
        return false;
    //if (input_keys.length >= max_input_keys)
    //    return false;
    // 特殊字根符號，如 ~en, ~sym
    if (v.charAt(0) == '~' ||
        (input_keys.length > 0 && input_keys[0].charAt(0) == '~') )
    {
        // 當輸入特殊字根(~)，或已輸入鍵的第一個是特殊字根時，皆直接替換整組輸入鍵
        // 1. 輸入~: 直接跳特殊字根候選字
        // 2. 已輸入鍵的第一個是特殊字根: 直接跳原輸入法的字根候選字
        input_keys = [v];
    }
    else {
        if (input_keys.length >= max_input_keys)
            return false;
        input_keys.push(v);
    }

    refresh_show_input_keys();
    refresh_candidates();
    return true;
}

function select_output_texts() {
    output.select();
    //console.log(output.isContentEditable);
    if (!document.queryCommandSupported('copy'))
        return;
    try {
        document.execCommand('copy', 'text/plain', null);
    }
    catch (e) {/*nothing.*/}
}

function clear_output_texts() {
    output.value = '';
    clear_input_keys();
}

function backspace_output_texts() {
    output.focus(); // 讓使用者注意動作位置
    // execCommand('delete') not work with input control. see issue 2.
    /*
    // normal input control has contentEditable attrib and value is true.
    if (document.queryCommandSupported('delete') && output.nodeName.toLowerCase() == 'div' && output.contentEditable) {
        document.execCommand('delete', false, null);
    }
    else */
    /*
    The selectionEnd attribute must, on getting, return the offset (in logical
    order) to the character that immediately follows the end of the selection.
    If there is no selection, then it must return the offset (in logical order)
    to the character that immediately follows the text entry cursor.
    W3 HTML5 - 4.10 Forms. */
    if (output.value.length < 1)
        return false;
    var txt = output.value;
    var entry_cursor = output.selectionEnd;
    if (entry_cursor != undefined) {
        if (entry_cursor < 1)
            return false;
        output.value = txt.substring(0, entry_cursor - 1).
            concat(txt.substring(entry_cursor));
        output.selectionStart = output.selectionEnd = entry_cursor - 1;
    }
    else
        output.value = output.value.substring(0, output.value.length - 1);
}

function add_char_output_texts(ch) {
    output.focus(); // 讓使用者注意出字到什麼地方
    // execCommand('delete') not work with input control. see issue 2.
    /*if (document.queryCommandSupported('insertText') && output.nodeName.toLowerCase() == 'div' && output.contentEditable) {
        console.log('insertTExt');
        document.execCommand('insertText', false, ch);
    }
    else */
    var txt = output.value;
    var entry_cursor = output.selectionEnd;
    if (entry_cursor != undefined) {
        output.value = txt.substring(0, entry_cursor).
            concat(ch, txt.substring(entry_cursor));
        output.selectionStart = output.selectionEnd = entry_cursor + ch.length;
    }
    else
        output.value += ch;
}

function add_space_output_texts() {
    add_char_output_texts(' ');
}

function add_newline_output_texts() {
    add_char_output_texts('\n');
}

function back_input_key() {
    input_keys.pop();
    refresh_show_input_keys();
    refresh_candidates();
}

function clear_input_keys() {
    input_keys = [];
    refresh_show_input_keys();
    refresh_candidates();
}

// Older browsers may not implement classList.
function class_list_add(node, class_name) {
    if (classList_supporting)
        node.classList.add(class_name);
    else
        node.className += ' ' + class_name;
}

function class_list_toggle(node, class_name) {
    if (classList_supporting /*&& node.classList.toggle*/)
        return node.classList.toggle(class_name);
    //console.info("browser does not support classList.toggle");
    var classes = node.className.split(' ');
    var need_to_turn_on = true;
    classes.forEach(function(v, i, oa){
        if (v == class_name) {
            oa.splice(i, 1); // toggle off
            need_to_turn_on = false;
        }
    });

    if (need_to_turn_on)
        classes.push(class_name);
    node.className = classes.join(' ');
    return need_to_turn_on;
}

function class_list_contains(node, class_name) {
    if (classList_supporting /*&& node.classList.contains*/)
        return node.classList.contains(class_name);
    //console.info("browser does not support classList.contains");
    var classes = node.className.split(' ');
    for (var i = 0; i < classes.length; ++i) {
        if (classes[i] == class_name)
            return true;
    }
    return false;
}

function toggle_auto_clear_input_keys() {
    auto_clear_input_keys = !auto_clear_input_keys;
    var elms, i;
    elms = document.getElementsByClassName(control_classes['auto_clear_input_keys_mode']);
    if (elms.length == 0)
        return;
    //auto_clear_input_keys = class_list_contains(elms[0], control_classes['auto_clear_input_keys_on']);
    for (i = 0; i < elms.length; ++i) {
        if (typeof control_classes['auto_clear_input_keys_on'] == 'function')
            control_classes['auto_clear_input_keys_on'].call(elms[i], auto_clear_input_keys);
        else
            class_list_toggle(elms[i], control_classes['auto_clear_input_keys_on']);
    }
}

function toggle_capital() {
    capital_mode = !capital_mode;
    var elms, i, kb_text;
    elms = document.getElementsByClassName(control_classes['capital_toggle']);
    if (elms.length == 0)
        return;
    //capital_mode = class_list_contains(elms[0], control_classes['capital_toggle_on']);
    for (i = 0; i < elms.length; ++i) {
        if (typeof control_classes['capital_toggle_on'] == 'function')
            control_classes['capital_toggle_on'].call(elms[i], capital_mode);
        else
            class_list_toggle(elms[i], control_classes['capital_toggle_on']);
    }

    if (TouchInputMethod.oncapitalization)
        TouchInputMethod.oncapitalization(capital_mode);
    console.log("capital_mode: ", capital_mode);

    var kbs = keyboard.getElementsByTagName('button');
    var ikb;
    for (ikb = 0; ikb < kbs.length; ++ikb) {
        kb_text = kbs[ikb].textContent;
        if (capital_mode && kb_text >= 'a' && kb_text <= 'z')
            kbs[ikb].textContent = kb_text.toUpperCase()
        else if (!capital_mode && kb_text >= 'A' && kb_text <= 'Z')
            kbs[ikb].textContent = kb_text.toLowerCase()
        /* // 減少改寫 DOM 內容的次數
        kbs[ikb].textContent = (capital_mode ?
            kbs[ikb].textContent.toUpperCase() :
            kbs[ikb].textContent.toLowerCase());
        */
    }
    kbs = kb_text = false;
}

function refresh_show_input_keys() {
    if (!show_input_keys)
        return;
    var keys = [];
    for (var i = 0; i < input_keys.length; ++i) {
        keys.push(key_sym_map[input_keys[i]]);
    }
    show_input_keys.textContent = (keys.length > 0 ? keys.join('') : ' ');
}

function trim(s) {
    var h, t;
    for (h = 0; h < s.length; ++h) {
        if (s.charCodeAt(h) > 32)
            break;
    }
    for (t = s.length; t > h; --t) {
        if (s.charCodeAt(t) > 32)
            break;
    }
    return s.substring(h, t+1);
}

// usually be invoked by refresh_candidates().
function update_candidates(data) {
    if (!candidate)
        return true;
    if (input_keys.length < 1) {
        candidate.innerHTML = '';
        return true;
    }

    // 避免單一 ',' 字元。
    var candidates = (data.length == 1 ? [data] : data.split(','));

    if (input_keys.length == max_input_keys && candidates.length == 1) {
        // 已輸入最多字根且只有一個候選字，自動出字
        add_char_output_texts(capital_mode ? candidates[0].toUpperCase() : candidates[0]);
        clear_input_keys();
        return;
    }

    candidate.innerHTML = ''
    candidates.forEach(function(v, idx, ot) {
        if (v.length < 1)
            return;
        //v = v.replace(/^\s+|\s+$/g, '', ''); // trim non-printable
        v = trim(v);
        if (v.length < 1)
            return;
        if (capital_mode)
            v = v.toUpperCase();
        candidate.insertAdjacentHTML('beforeend',
            '<button class="'.concat(control_classes['candidates'], '">', v, '</button>'));
    });
    for (var elms = document.getElementsByClassName(control_classes['candidates']), i = 0;
         i < elms.length;
         ++i)
    {
        elms[i].addEventListener('click', function(){
            add_char_output_texts(this.textContent);
            if (auto_clear_input_keys)
                clear_input_keys();
        });
    }
} // end update_candidates()

function refresh_candidates() {
    if (input_keys.length < 1) {
        update_candidates('');
        return true;
    }

    var table_key;
    table_key = input_keys.join('');
    //console.log("key: ", table_key);

    var data = false;
    if (table_key.charAt(0) == '~') { //獨立字根，一律從 localStorage 或 .tab 中讀取。
        table_key = table_key.substring(1); // strip '~'
        var local_table_key = 'input_method_'.concat(table_key);
        console.log("search localStorage by key ", local_table_key);
        if (localStorage)
            data = localStorage.getItem(local_table_key);
        if (!data) {
            console.log("not in localStorage");
            var text_table_name = table_key.concat('.tab');
            console.log("get table_name: ", text_table_name);
            table_xhr.open('GET', resources_path+text_table_name, false); // sync call.
            try {
                table_xhr.send();
                if (table_xhr.status < 300)
                    data = table_xhr.responseText;
                //console.log("load ", data.substring(0, 20));
            }
            catch (e) {
                data = false;
                console.info("could not request " + text_table_name);
                return false;
            }
            if (data && localStorage) {
                console.log("save in localStorage");
                localStorage.setItem(local_table_key, data);
            }
            else
                console.info("could not request " + text_table_name);
        }
    }
    else if (word_table) {
        data = word_table[table_key];
        if (data) {
            //console.log("words: ", data);
        }
        else {
            console.info('no such table, skip'); // key: 4-3-2
            return false;
        }
    }
    /*
    else {
        data = localStorage.getItem(table_name);
        //console.log('localStorage: %s', data);
    }
    */

    if (data) {
        update_candidates(data);
        return true;
    }

    /*
    var text_table_name = 'tbl/' + input_keys.join('-') + '.tab';
    //console.log("table_name: %s", text_table_name);
    table_xhr.open('GET', text_table_name, true);
    try {
        table_xhr.send();
    }
    catch (e) {
        // ignore.
    }
    */
    return true;
} // end refresh_candidates()

function bind_all_text_input_focus_handler() {
    var inputs = document.querySelectorAll('input, textarea');
    for (var i = 0; i < inputs.length; ++i) {
        /*
        if (this.type in special_input_keyboard_map &&
            special_input_keyboard_map[this.type] == 'ignore')
        {
            continue;
        }
        */
        inputs[i].addEventListener('focus', function() {
            if (output != this) {
                output = this;
                according_input_type_switch_engine();
                compositing = false; // let it invoke oncomposition().
            }

            if (typeof TouchInputMethod.oncomposition == 'function' && !compositing) {
                compositing = true;
                TouchInputMethod.oncomposition();
            }
        }, false);
    }
} // end bind_all_text_input_focus_handler()

}); // end var TouchInputMethod

document.addEventListener('DOMContentLoaded', TouchInputMethod.init, false);

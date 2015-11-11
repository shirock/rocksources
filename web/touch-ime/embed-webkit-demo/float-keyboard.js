    var float_ime_div = false;
    console.log('init_float_ime_div');
    var innerHtml = '<style type="text/css">\
    #ime_area { \
        visibility: hidden; \
        position: absolute; \
        z-index: 1000000; \
        bottom: 0; \
        width: 440px;} \
    #input_method_show_input_keys { height: 1em; } \
    #input_method_candidate { \
        height: 50px; max-height: 50px; \
        overflow: auto; \
    } \
    .input_method_inputkey { width: 2em; height: 2em; } \
    .input_method_capital_toggle_on { background-color: red; } \
    </style>\
    <div>\
        <button class="input_method_back_input_key">退一字根</button>\
        <button class="input_method_backspace_output_texts">回刪一字</button>\
        <button class="input_method_add_space_output_texts">空格</button>\
        <button class="input_method_add_newline_output_texts">換行</button>\
    </div>\
    <div>\
        <button class="input_method_goto_next">下一欄</button>\
        <button class="input_method_goto_previous">上一欄</button>\
    </div>\
    <div>\
        <button class="input_method_capital_toggle">Caps</button>\
        <select class="input_method_select_engine">Switch</select>\
        <button class="input_method_end_composition">隱藏鍵盤</button>\
    </div>\
    <div id="input_method_keyboard"></div>\
    <div id="input_method_show_input_keys"></div>\
    <div id="input_method_candidate"></div>';

    float_ime_div = document.createElement('div');
    float_ime_div.id = 'ime_area';
    float_ime_div.innerHTML = innerHtml;
    document.getElementsByTagName('body')[0].appendChild(float_ime_div);



```text
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

You should see https://rocksources.googlecode.com/ to get more information about Touch IME.
```
<img src="http://www.gnu.org/graphics/lgplv3-147x51.png" alt="LGPLv3"/>

<!--more-->

## 概觀

TouchInputMethod (以下稱 *touch-ime* ) 是針對純觸控螢幕、無鍵盤環境的 HTML5 應用軟體設計的輸入法引擎。

使用特徵:

* 焦點進入輸入框時，便會出現鍵盤。一般設計時亦可採固定顯示。
* touch-ime 會記憶最近使用的輸入框。就算使用者碰觸其他區域使焦點離開輸入框， touch-ime 還是會把輸入內容填入預期的輸入框。
* 點擊鍵盤上的「結束輸入法」鍵，則會收起鍵盤。
* touch-ime 可以在失去焦點的狀態下，繼續輸入工作。所以不會以「失去焦點」為時間點自動收起浮動鍵盤。
* 選取特定型態輸入框時 (password, mail) ，會出現對應型態的鍵盤佈局。
* touch-ime 會記憶單純文字型態 (text) 的輸入框使用的輸入法引擎，而不是每次都使用預設輸入法引擎。
* 可擴展的輸入法引擎以及可自定的鍵盤佈局。
* 可自定詞庫與常用符號表。

目前提供下列輸入法與鍵盤佈局:

* 英數: en
* 行列30: array30
* 注音: zhuyin
* 拼音 (漢語拼音): pinyin
* 倉頡速成 (快速倉頡): quick
* 倉頡三代: cangjie3
* 電子郵件型態輸入元件適用的鍵盤佈局: email

source repo: [https://github.com/shirock/rocksources/tree/master/web/touch-ime](https://github.com/shirock/rocksources/tree/master/web/touch-ime)

源碼安裝: 你需要下載所有的 .js 文件和 .tab 文件。

範例文件:

* index.html - 一般使用範例。由頁面設計者安排輸入法控制項的位置。
* offline.html - 離線使用範例。如何透過 Cache 機制，在不連接網路的狀態下使用 touch-ime 。
* on-screen-keyboard-demo.html - 配合 touch-ime 的 OSD Keyboard 範例。

deb安裝: 安裝 touch-ime 。檔案將被安裝在 /usr/share/javascript/touch-ime 。以瀏覽器開啟範例文件，例如 file:///usr/share/javascript/touch-ime/index.html (部份瀏覽器不允許開啟 file:// 協定) 。

參考: [Touch IME 介紹](http://rocksaying.tw/archives/25452026.html)。

### 使用情境 (index.html)

畫面上有4個輸入項目，使用者點取第一個文字輸入區，並切換到速成輸入法輸入。

![範例圖 - 中文輸入](https://ajzhig.blu.livefilestore.com/y2pEtJfVzRu8xUDlbNqRYMA6ajWlJwLmQtY1PlB42l185G0thRuT_T-bTQhpbjYI52tbUfiYnQklb3HFK8hMnALxWGzGGWeFl2XDf041Q0h2fk/%E5%BF%AB%E7%85%A719.png?psid=1)

使用者點取 pass 欄位，因為該欄位的型態是 password ，故 touch-ime 自動切換到英數輸入。

![範例圖 - password](https://w2uk4a.blu.livefilestore.com/y2pz76AJ_148t6zNWtr8HRBEHMJiNtONhFw8UPXTjXc69kzQCg32jZULWyi2SuJKVE66p906DO4y-bC9OQL1v_IkTmLVXE5sxpg83nqZ4yL4hM/%E5%BF%AB%E7%85%A720.png?psid=1)

使用者點取 mail 欄位，因為該欄位的型態是 email ，故 touch-ime 自動切換到電郵輸入。

![範例圖 - email](https://w2slvq.blu.livefilestore.com/y2pQuap5Xg3L-9l8V9cQKuo5fN88i_gRho7PXSXDEI-U2T5gXKaTe-VivxcYSuh_PYDQZ0zzN3wLbkVMMmUNvjxtJL057TUJBseiQdzHxd9iDw/%E5%BF%AB%E7%85%A721.png?psid=1)

使用者點擊 Caps (大寫變換) 輸入法控制項，鍵盤區鍵面符號變大寫，出字也變大寫。

![範例圖 - Caps](https://ajxgrg.blu.livefilestore.com/y2pBe254z-7AALxZ2VaqytoxELcEIoCTdBjwdKK5pHLCbb3SyF3IiEWtZdCyonm5NcP25cMVb1a9R7xCdbFpx8NNUaBmX6n-AoDGOF2Rytb-RQ/%E5%BF%AB%E7%85%A722.png?psid=1)


### 用例

{% highlight html %}
<script src="touch-ime.js" type="text/javascript">
</script>

<input type="text" />
<input type="email" />
<textarea>
</textarea>

<div><!-- optional -->
    <button class="input_method_back_input_key">退一字根</button>
    <button class="input_method_clear_input_keys">清除字根</button>
    <button class="input_method_auto_clear_input_keys_mode">自動清除字根</button>
    <button class="input_method_capital_toggle">Caps Lock</button>
    <button class="input_method_backspace_output_texts">回刪一字</button>
    <button class="input_method_clear_output_texts">清除輸入內容</button>
    <button class="input_method_add_space_output_texts">空格</button>
    <button class="input_method_add_newline_output_texts">換行</button>
    <button class="input_method_switch_engine">切換</button>
    <button class="input_method_end_composition">結束輸入法</button>
</div>

<div id="input_method_keyboard"><!-- required -->
</div>

<div id='input_method_show_input_keys'><!-- required -->
</div>

<div id='input_method_candidate'><!-- required -->
</div>

{% endhighlight %}


### 何時需要 touch-ime

基本上，如果你的 HTML5 應用軟體是執行在移動式環境的作業系統，例如 Firefox OS 、 Tizen 、Windows Phone 等。它們本身就已經提供了完善的輸入法系統，你不需要 touch-ime 。

再者，若你的 HTML5 應用軟體執行在客製化的 Linux 桌面環境，那麼你首先應該考慮的輸入方案是使用 On-Screen 鍵盤 (軟體虛擬鍵盤) 。On-Screen 鍵盤可以輕易地和系統內建的輸入法系統，如 IBus, SCIM 等互動，達成使用者輸入的需求。這也是 [http://www.w3.org/TR/ime-api/ W3C Input Method Editor API] 目前建議的方式。

唯有你的系統不提供你需要的中文輸入法，或者你無法操控 On-Screen 鍵盤時，你才需要考慮使用 touch-ime 。


## 基本說明

1. 載入 Touch 輸入法的程式碼: touch-ime.js (以下稱 touch-ime)。
2. 提供三個 touch-ime 要用的 div 。
3. 以 style class 指定其他輸入法控制項。

基本上，你只需要載入 touch-ime.js 並確保你的 HTML 中有下列 3 個必要的 div ，touch-ime 就可運作了。但缺少選用性的輸入法控制項目時，例如切換輸入法、刪字、空格等，使用者會覺得相當不便。所以建議你應考慮畫面佈局，安排輸入法控制項。

touch-ime 不會決定輸入法鍵盤區在你畫面上的佈局或位置，也不會自己用突現式或漂浮式的呈現形式出現。它需要的鍵盤區、候選字區、控制項區的位置，交由設計者(也就是你)安排。還有，輸入法控制項的外觀，也交由設計者以 CSS 設計。

### touch-ime 必要的 div 預設 id

* input_method_keyboard : 輸入法鍵盤區。
* input_method_candidate : 輸入法候選字區。候選字數可能超過可見範圍，故應允許此區域可以捲動內容，以便使用者滑動選字區內容挑字。
* input_method_show_input_keys: 輸入法已輸入字根的顯示區。

以上三區是必要項目，而且必須是唯一的，所以用 id 識別。

### touch-ime 選用性的輸入法控制項的預設 style class

* input_method_inputkey :                   字根鍵盤外觀。
* input_method_candidates :                 候選字外觀。
* input_method_back_input_key :             「退一字根」控制項與外觀。
* input_method_clear_input_keys :           「清除字根」控制項與外觀。
* input_method_auto_clear_input_keys_mode : 「自動清除字根」控制項與外觀。
* input_method_auto_clear_input_keys_on :   自動清除功能鈕啟動時外觀。
* input_method_capital_toggle :             「Caps」控制項與外觀。
* input_method_capital_toggle_on :          Caps 啟動時外觀。
* input_method_backspace_output_texts :     「回刪一字」控制項與外觀。
* input_method_clear_output_texts :         「清除輸入內容」控制項與外觀。
* input_method_select_output_texts:         「選取輸入內容」控制項與外觀。
* input_method_add_space_output_texts :     「空白」控制項與外觀。
* input_method_add_newline_output_texts :   「換行」控制項與外觀。
* input_method_switch_engine :              「切換輸入法」控制項與外觀。
* input_method_select_engine :              「選擇輸入法」控制項與外觀。這必須是 select 控制項。
* input_method_goto_next:                   「移到下一欄位」控制項與外觀。
* input_method_goto_previous:               「移到上一欄位」控制項與外觀。
* input_method_end_composition :            「結束輸入法」控制項與外觀。

以上輸入法控制項是可選用的，可有可無。甚至同樣功能的控制項也可以安排兩個或多個，所以它們用 style class 識別。


## 自訂功能

### TouchIME 方法與屬性

進階自訂項目需要使用 JavaScript 操作。

#### 指定輸入法控制項 set_controls

touch-ime 提供 `TouchInputMethod.set_controls()` 方法讓設計者自訂控制項的 id 和 style class 。

{% highlight javascript %}

TouchInputMethod.set_controls(custom_keyboard_id, custom_candidate_id,
  custom_show_input_keys_id, optional_classes);

{% endhighlight %}

三個必要的 div id ，若給 `null` 表示用預設值。

<var>optional_classes</var> 為 Hash table，用於自訂輸入法控制項的 style class 名稱。可用的參數項如下列:

* inputkey                      字根鍵盤外觀的 style class 名稱。
* candidates                    候選字外觀的 style class 名稱。
* back_input_key                「退一字根」控制項與外觀的 style class 名稱。
* clear_input_keys              「清除字根」控制項與外觀的 style class 名稱。
* auto_clear_input_keys_mode    「自動清除字根」控制項與外觀的 style class 名稱。
* auto_clear_input_keys_on      自動清除功能鈕啟動時外觀的 style class 名稱。
* capital_toggle                「Caps」控制項與外觀的 style class 名稱。
* capital_toggle_on             Caps 啟動時外觀的 style class 名稱。
* backspace_output_texts        「回刪一字」控制項與外觀的 style class 名稱。
* clear_output_texts            「清除輸入內容」控制項與外觀的 style class 名稱。
* select_output_texts:          「選取輸入內容」控制項與外觀的 style class 名稱。
* add_space_output_texts        「空白」控制項與外觀的 style class 名稱。
* add_newline_output_texts      「換行」控制項與外觀的 style class 名稱。
* switch_engine                 「切換輸入法」控制項與外觀的 style class 名稱。
* select_engine                 「選擇輸入法」控制項與外觀的 style class 名稱。這必須是 select 控制項。
* goto_next:                    「移到下一欄位」控制項與外觀的 style class 名稱。
* goto_previous:                「移到上一欄位」控制項與外觀的 style class 名稱。
* end_composition               「結束輸入法」控制項與外觀的 style class 名稱。

#### get_controls

get css selector (id and class) of controls.


#### 指定組字結果的輸出目標控制項 set_target

`TouchInputMethod.set_target()` 方法可指定組字結果的輸出目標控制項。

{% highlight javascript %}

TouchInputMethod.set_target(dom);

{% endhighlight %}


參數 <var>dom</var> 必須是一個 `DOMInputElement` 。

touch-ime 在載入時會自動綁定頁面上已有的輸入控制項，以偵測輸出目標。
但是，若設計者在頁面載入後才透過 JavaScript 新增輸入控制項，這些後來才增加的輸入控制項就不會為 touch-ime 所知。
此時設計者需要自行呼叫 `set_target()` 方法告知 touch-ime 將組字結果輸出到哪個控制項。

#### 取得目前的輸出目標控制項 get_target

`TouchInputMethod.get_target()` 方法可取得目前的輸出目標控制項。

{% highlight javascript %}

TouchInputMethod.get_target();

{% endhighlight %}


回傳一個 `DOMInputElement` 。

不要用這個方法取得目前焦點所在的控制項，結果可能不同你預期。

#### 指定資源目錄路徑 set_resources_path

{% highlight javascript %}

TouchInputMethod.set_resources_path(resources_folder_path);

{% endhighlight %}


預設是從 touch-ime.js 相同目錄下載入字根表等內容。

請注意瀏覽器的相同來源政策，如果你的字表資源放在不同來源伺服器，那麼你必須按照 W3 CORS 的建議，設置跨來源資源分享規則。若未設置 CORS ，那麼你在此只能指定資源目錄的路徑。例如 '/js/touch-ime/' 。

參考:

* http://www.w3.org/TR/cors/
* http://en.wikipedia.org/wiki/Cross-origin_resource_sharing

#### 啟用輸入法引擎 enable_engines

指定啟用哪些輸入法引擎。

{% highlight javascript %}

TouchInputMethod.enable_engines(engine_code_list);

{% endhighlight %}


參數 <var>engine_code_list</var> 是一個列出輸入法引擎代碼名的陣列。

目前有效的代碼名:

* zhuyin 注音
* pinyin 拼音
* en 英數
* enShift 英數大寫
* email 電子郵件
* array30 行列30
* cangjie3 倉頡第三代
* cangjie5 倉頡第五代
* quick 倉頡速成

預設啟用 zhuyin, en, email, pinyin, quick, array30 。

#### 改變目前的輸入法引擎 change_engine

改變目前的輸入法引擎。

{% highlight javascript %}

TouchInputMethod.change_engine(engine_code);

{% endhighlight %}


參數 <var>engine_code</var> 必須是一個有效的輸入法引擎代碼名。如果無法切換指定的輸入法引擎，則回傳 `false` 。

#### 指定控制項自動適用的輸入法引擎 specify_control_with_engine

{% highlight javascript %}

TouchInputMethod.specify_control_with_engine(control_class, engine_code);

{% endhighlight %}


<var>control_class</var> 表示控制項的 class 或 type 。若 <var>engine_code</var> 為 `false` ，則移除適用規則。

touch-ime 預設下列 type 控制項適用對象:

* 'password': 'en'
* 'datetime': 'en'
* 'date': 'en'
* 'month': 'en'
* 'week': 'en'
* 'time': 'en'
* 'localdatetime': 'en'
* 'number': 'en'
* 'range': 'en'
* 'email': 'email'

除了這些預設對象，設計者還可利用 `specify_control_with_engine()` 指定控制項的適用輸入法。其規則為:

1. 如果控制項的 class 清單中 **包含** 了 control_class ，則適用指定的輸入法。  大部份設計者使用這個規則。
2. 如果控制項的 type **等於** control_class ，則適用指定的輸入法。  基本上 touch-ime 預設的適用表已符合使用情境，不需要使用此規則改變。
3. 以上皆非，則適用使用者手動選擇的輸入法。  手動選擇是指使用者透過「切換輸入法」、「選擇輸入法」或調用 `change_engine()` 選擇輸入法的行為。

當使用者點取文字輸入控制項時， touch-ime 就會根據適用規則，自動切換輸入法，避免使用者自己切換輸入法的困擾。

#### 清除本地儲存的特殊字表 clean_local_table

{% highlight javascript %}

TouchInputMethod.clean_local_table();

{% endhighlight %}


自本地儲存區 (`localStorage`) 中清除下列特殊字表的內容。

* en.tab (localStorage 名稱 input_method_en) 英數字元表。
* s.tab (localStorage 名稱 input_method_s) 詞庫表。
* w.tab (localStorage 名稱 input_method_w) 全形符號表。
* ascii.tab (localStorage 名稱 input_method_ascii) 半形符號表。

touch-ime 會讀取特殊字表檔的內容並保存在本地儲存區。如果你修改了特殊字表檔的內容，則你必須調用 `clean_local_table()` 清除已儲存的內容，touch-ime 才會重新讀取新的特殊字表檔內容(並再次保存於本地儲存區)。

#### 組字事件 oncomposition

`TouchInputMethod.oncomposition` 屬性可指定組字時的處理行為。

當使用者點取可以輸入文字的頁面控制項時，就會觸發此事件。

#### 結束組字事件 oncompositionend

`TouchInputMethod.oncompositionend` 屬性可指定結束組字時的處理行為。

只有使用者按下 end_composition 控制項時才會觸發此事件。

#### 大寫變換模式事件 oncapitalization

此事件會傳入一個參數告知目前的大寫變換模式狀態。


### 加入更多的輸入法

若要擴充輸入法內容，需要編輯輸入法表格。

每一個輸入法都需要兩個表格，一為字根，一為鍵盤配置。分成 `$PREFIX-tbl.js` 和 `$PREFIX-keyboard.js` 。皆為 JSON 格式。

#### 字根表 -tbl.js

字根為鍵，候選字為值，多個候選字間以逗號','分隔。

範例:

{% highlight javascript %}

{
    "1": "!,~",
    "2": "@",
    .
    .(省略)
    .
    "enter": "\n"
}
{% endhighlight %}


#### 鍵盤表 -keyboard.js

需具備三個鍵:

* max_input_keys  型態為整數，指定此輸入法引擎最長的字根長度。預設為 4 。
* key_sym_map  型態為  hash table，指定字根值與鍵盤上顯示的符號。
* keyboard_layout  型態為陣列，表示鍵盤排列方式。以字根值為主，"\n" 表示換行排列。

範例:

{% highlight javascript %}
{
"max_input_keys": 1,

"key_sym_map": {
  "1": "!",
  "2": "@",
  .
  .(省略)
  .
  "enter": "↙"
}

},

"keyboard_layout": [
  "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "minus", "\n",
  "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "plus", "\n",
  "a", "s", "d", "f", "g", "h", "j", "k", "l", "slash", "\n",
  "z", "x", "c", "v", "b", "n", "m", "<", ">", "enter"
]
}
{% endhighlight %}


#### 特殊字表

以下三個字表檔與輸入法引擎無關，屬於通用項目。

* en.tab  英數字元表。字根為 '~en' 。
* s.tab  自定的常用詞表。字根為 '~s' 。
* w.tab  標點符號表。字根為 '~w' 。

字根若為 '~' 開頭者，就視為特殊字表用字根。輸入法引擎製表時請勿使用此字元作為字根。


## 進階用例

載入 touch-ime.js 後，加上 JavaScript 程式碼，於程式碼中調用 touch-ime 提供的設定內容。

{% highlight javascript %}

<script src="touch-ime.js" type="text/javascript">
</script>
<script>
TouchInputMethod.set_resources_path('/js/touch-ime/');
// 指定字表檔的路徑。

TouchInputMethod.set_controls(
    'my_ime_keyboard',
    'my_ime_candidate',
    'my_ime_show_codes',
    {
        'clear_input_keys': 'my_ime_clear_input_codes',
        'backspace_output_texts': 'my_ime_backspace',
        'add_space_output_texts': 'my_ime_add_space',
        'add_newline_output_texts': 'my_ime_add_newline',
        'end_composition': 'my_ime_end_composition'
    }
);

TouchInputMethod.enable_engines(["en", "enShift", "zhuyin", "quick"]);
// 啟用英數、注音和倉頡速成。

TouchInputMethod.specify_control_with_engine("phone_number", "en");
// 指定 class 包含 'phone_number' 的控制項適用 en 輸入。

TouchInputMethod.oncomposition = function() {
    console.log("show TouchInputMethod");
    document.getElementById('my_ime_area').style.visibility = "visible";
}

TouchInputMethod.oncompositionend = function() {
    console.log("hide TouchInputMethod");
    document.getElementById('my_ime_area').style.visibility = "hidden";
}
</script>

<input type="text" />
<input class="phone_number" type="text" />
<input type="email" />
<textarea>
</textarea>

<div id="my_ime_area" style="visibility:hidden;">
    <div><!-- optional -->
        <button class="my_ime_clear_input_codes">清除字根</button>
        <button class="my_ime_backspace">回刪一字</button>
        <button class="my_ime_add_space">空格</button>
        <button class="my_ime_add_newline">換行</button>
        <button class="my_ime_end_composition">結束輸入法</button>
    </div>

    <div id="input_method_keyboard"><!-- required -->
    </div>

    <div id='input_method_show_input_keys'><!-- required -->
    </div>

    <div id='input_method_candidate'><!-- required -->
    </div>
</div>

{% endhighlight %}


### 離線應用

若需要離線應用，請參考 touch-ime.manifest ，將其列出的檔名內容，加入你的快取清單中。

欲使快取清單生效，需要下列條件配合:

一、以 Apache http server 為例，需要在 mime.types 中加入下列設置:

{% highlight text %}

text/cache-manifest  manifest

{% endhighlight %}


mime.types 的路徑，依你的 Apache 設置而定。可能位在 /etc/httpd/mime.types 、 /etc/apache2/mime.types 、或 /etc/mime.types 。

二、在你的網頁的 html 標籤中，增加  manifest 屬性，指出你的快取清單檔名。例如:

{% highlight html %}

<html manifest="app.manifest">
 .
 . (省略)
 .
</html>

{% endhighlight %}


實際的離線應用案例，可參考「[在 Windows Phone 8 上離線使用 Touch IME](http://rocksaying.tw/archives/25709686.html)」。

### 附加 On Screen Keyboard

on-screen-keyboard.js 是一個以 HTML5 規格實作的軟體虛擬鍵盤 (OSD keyboard)，故只能用於 HTML5 應用之內。嚴格說來它只是 On page display keyboard ，但為了方便查找，還是按正式說法的 On screen display keyboard 命名。

原本 touch-ime 的設計哲學是由 UI 設計者自行規劃輸入法與鍵盤的控制項佈局。不過針對 Embedded webkit 這類型的應用，若有一個可以無縫置入的軟體虛擬鍵盤會比較方便。所以我增加了這個附加項目。

on-screen-keyboard.js 的使用方式很簡單，將它放在 touch-ime.js 的後面即可。請看以下範例:

{% highlight html %}

<script src="touch-ime.js" type="text/javascript"></script>
<script type="text/javascript">
// Put you touch-ime setup if you need.
</script>

<script src="on-screen-keyboard.js" type="text/javascript"></script>

<body>
Your page content.
</body>
{% endhighlight %}


on-screen-keyboard.js 會在頁面載入完成後，安插一個隱藏的虛擬軟鍵盤節點，以便讓 touch-ime 使用。然後 on-screen-keyboard.js 會去傾聽 touch-ime 的 oncomposition 和 oncompositionend 事件，決定顯示或隱藏虛擬軟鍵盤。這個虛擬軟鍵盤懸浮在頁面上，使用者可以拖動其位置，以免其遮擋到頁面資訊。這些動作都是 on-screen-keyboard.js 內部處理負責的，程式設計人員基本上不必關心它。事實上，由於 on-screen-keyboard.js 的特殊設計，它是一個沒有名字可以指涉的黑箱，不開放任何行為給程式設計人員操作。它唯一開放的是它的源碼，所以想改變它就直接修改源碼吧。

on-screen-keyboard.js 具體的使用範例可以參考源碼中的 on-screen-keyboard-demo.html 或 「[Touch IME 使用在嵌入 WebKit 的場合](http://rocksaying.tw/archives/25651856.html)」。


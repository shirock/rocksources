/*
Taiwan 119 GPS Copyright (C) 2017 遊手好閒的石頭成 <shirock.tw@gmail.com>

Taiwan 119 GPS is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see
<http://www.gnu.org/licenses/gpl-3.0-standalone.html>.

You should see http://rocksaying.tw/ to get more information about this.
*/

// http://data.gov.tw/node/7917
const S119_API = 'http://od.moi.gov.tw/api/v1/rest/datastore/301060000C-000384-003';
var s119_list = { "success": true, "result": { "resource_id": "301060000C-000384-003", "limit": 2000, "total": 23, "fields": [{ "type": "text", "id": "unit" }, { "type": "text", "id": "SMS-number" }, { "type": "text", "id": "Alternate-SMS-number" }, { "type": "text", "id": "fax-number" }], "records": [{ "unit": "單位", "SMS-number": "簡訊號碼", "Alternate-SMS-number": "備用簡訊號碼", "fax-number": "傳真號碼" }, { "unit": "基隆市消防局", "SMS-number": "0911-511-901", "Alternate-SMS-number": "", "fax-number": "02-24294097" }, { "unit": "臺北市政府消防局", "SMS-number": "0932-299-702", "Alternate-SMS-number": "0963-330-119", "fax-number": "02-27587865" }, { "unit": "新北市政府消防局", "SMS-number": "0911-510-495", "Alternate-SMS-number": "", "fax-number": "02-89510905" }, { "unit": "桃園縣政府消防局", "SMS-number": "0911-511-904", "Alternate-SMS-number": "", "fax-number": "03-3371119" }, { "unit": "新竹市消防局", "SMS-number": "0911-511-905", "Alternate-SMS-number": "", "fax-number": "03-5260535" }, { "unit": "新竹縣政府消防局", "SMS-number": "0911-511-906", "Alternate-SMS-number": "", "fax-number": "03-5520331" }, { "unit": "苗栗縣政府消防局", "SMS-number": "0911-511-907", "Alternate-SMS-number": "", "fax-number": "037-271982" }, { "unit": "臺中市政府消防局", "SMS-number": "0911-511-908", "Alternate-SMS-number": "", "fax-number": "04-23820675" }, { "unit": "彰化縣消防局", "SMS-number": "0911-511-910", "Alternate-SMS-number": "", "fax-number": "04-7513126" }, { "unit": "南投縣政府消防局", "SMS-number": "0911-511-911", "Alternate-SMS-number": "", "fax-number": "049-2238034" }, { "unit": "雲林縣消防局", "SMS-number": "0911-511-912", "Alternate-SMS-number": "", "fax-number": "05-5351735" }, { "unit": "嘉義市政府消防局", "SMS-number": "0911-511-913", "Alternate-SMS-number": "", "fax-number": "05-2716638" }, { "unit": "嘉義縣消防局", "SMS-number": "0911-511-914", "Alternate-SMS-number": "", "fax-number": "05-3620359" }, { "unit": "臺南市政府消防局", "SMS-number": "0911-511-915", "Alternate-SMS-number": "", "fax-number": "06-2952154" }, { "unit": "高雄市政府消防局", "SMS-number": "0911-511-917", "Alternate-SMS-number": "", "fax-number": "07-8225911" }, { "unit": "屏東縣政府消防局", "SMS-number": "0911-511-919", "Alternate-SMS-number": "", "fax-number": "08-7655420" }, { "unit": "宜蘭縣政府消防局", "SMS-number": "0911-511-920", "Alternate-SMS-number": "", "fax-number": "03-9323175" }, { "unit": "花蓮縣消防局", "SMS-number": "0911-511-921", "Alternate-SMS-number": "", "fax-number": "03-8574962" }, { "unit": "臺東縣消防局", "SMS-number": "0911-511-922", "Alternate-SMS-number": "", "fax-number": "089-326923" }, { "unit": "澎湖縣政府消防局", "SMS-number": "0911-511-923", "Alternate-SMS-number": "", "fax-number": "06-9272457" }, { "unit": "金門縣消防局", "SMS-number": "0911-511-924", "Alternate-SMS-number": "", "fax-number": "082-371430" }, { "unit": "連江縣消防局", "SMS-number": "0919-919-995", "Alternate-SMS-number": "", "fax-number": "083-623816" }] } };

var coords = false;
var regions_ctrl = false;
var helpmsg_input = false;

function log(...msg) {
    console.log(...msg);
    //document.getElementById('console').innerHTML += msg.join(' ') + '<br/>';
}

function set_position_content(text) {
    document.getElementById('position').innerHTML = text;
}

function got_position(position, err=false) {
    let msg = '';
    if (position) {
        coords = position.coords;
    }
    else {
        msg = err + '<br/>使用原先位置<br/>';
    }
    if (!coords) {
        set_position_content('無法取得位置');
        return;
    }
    set_position_content(`${msg}緯度: ${coords.latitude}<br/>經度: ${coords.longitude}`);
    for (let p in coords) {
        log(`${p}: ${coords[p]}`);
    }
    /*
    let map = document.getElementById('map');
    let gmap_uri = "http://maps.googleapis.com/maps/api/staticmap?center=" +
        coords.latitude + "," +
        coords.longitude + "&zoom=13&size=300x300&sensor=false";
    map.href = gmap_uri;
    map.innerHTML = '查看地圖';
    let img = new Image();
    img.src = gmap_uri;
    map.appendChild(img); // This service requires an API key.
    */

    set_sms_content();

    document.getElementById('embedMap').src = `http://www.google.com.tw/maps?q=loc:${coords.latitude},${coords.longitude}&output=embed`;
}

function get_position() {
    log('get position');
    set_position_content('GPS 位置，偵測中...');
    navigator.geolocation.getCurrentPosition(
        got_position,
        (err) => {
            log(err);
            got_position(false, err);
        },
        {
            enableHighAccuracy: true,
            timeout: 5000, // 5 seconds
            maximumAge: 0
        }
    );
}

/*
+ RFC5724: SMS https://tools.ietf.org/html/rfc5724
+ RFC3966: telphone number
+ click to call: https://developers.google.com/web/fundamentals/native-hardware/click-to-call/
 */
function set_sms_content() {
    let position = (coords ? `GPS:${coords.latitude},${coords.longitude}` : '');
    let sms = regions_ctrl.options[regions_ctrl.selectedIndex].value;
    let msg = helpmsg_input.value.trim();
    let sms_href = `sms:${sms}?body=` +
        encodeURIComponent(`求救！ ${position} ${msg}`);
    log(sms_href);
    document.getElementById('sms').href = sms_href;
    localStorage.s119_sms = sms;
}

function helpmsg_changed() {
    set_sms_content();
    localStorage.s119_helpmsg = helpmsg_input.value.trim();
}

function create_regions() {
    let option;
    let default_sms = localStorage.s119_sms;

    regions_ctrl.innerHTML = '';
    s119_list.result.records.forEach((v, i) => {
        if (/^\D/.test(v['SMS-number']))
            return;
        //log(v.unit, v['SMS-number']);
        option = document.createElement('option');
        option.value = v['SMS-number'];
        option.textContent = v.unit;
        if (default_sms == v['SMS-number']) {
            option.selected = true;
        }
        regions_ctrl.appendChild(option);
    });
}

function init() {
    log('init...');

    regions_ctrl = document.getElementById('regions');
    helpmsg_input = document.getElementById('helpmsg');

    if (localStorage.s119_list)
        s119_list = JSON.parse(localStorage.s119_list);

    if (localStorage.s119_helpmsg)
        helpmsg_input.value = localStorage.s119_helpmsg;

    // try to update list from internet.
    var xhr = new XMLHttpRequest();
    if (xhr.overrideMimeType)
        xhr.overrideMimeType('application/json');
    xhr.addEventListener('load', function () {
        if (this.status >= 300)
            return;
        //log(this.responseText);
        log("refresh fd list");
        let fd_list = JSON.parse(this.responseText);
        if (fd_list.success) {
            localStorage.s119_list = this.responseText;
            s119_list = fd_list;
            create_regions();
        }
    });
    xhr.addEventListener('error', function (evt) {
        log(evt);
    });
    xhr.open('GET', S119_API, true);
    xhr.send();

    create_regions();
    get_position();
    set_sms_content();

    document.getElementById('get-position').addEventListener('click', get_position);
    regions_ctrl.addEventListener('change', set_sms_content);
    helpmsg_input.addEventListener('change', helpmsg_changed);
}

document.addEventListener('DOMContentLoaded', init, false);

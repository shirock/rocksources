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
var u119_api = 'http://od.moi.gov.tw/api/v1/rest/datastore/301060000C-000384-003';
var u119_list = {"success":true,"result":{"resource_id":"301060000C-000384-003","limit":2000,"total":23,"fields":[{"type":"text","id":"unit"},{"type":"text","id":"SMS-number"},{"type":"text","id":"Alternate-SMS-number"},{"type":"text","id":"fax-number"}],"records":[{"unit":"單位","SMS-number":"簡訊號碼","Alternate-SMS-number":"備用簡訊號碼","fax-number":"傳真號碼"},{"unit":"基隆市消防局","SMS-number":"0911-511-901","Alternate-SMS-number":"","fax-number":"02-24294097"},{"unit":"臺北市政府消防局","SMS-number":"0932-299-702","Alternate-SMS-number":"0963-330-119","fax-number":"02-27587865"},{"unit":"新北市政府消防局","SMS-number":"0911-510-495","Alternate-SMS-number":"","fax-number":"02-89510905"},{"unit":"桃園縣政府消防局","SMS-number":"0911-511-904","Alternate-SMS-number":"","fax-number":"03-3371119"},{"unit":"新竹市消防局","SMS-number":"0911-511-905","Alternate-SMS-number":"","fax-number":"03-5260535"},{"unit":"新竹縣政府消防局","SMS-number":"0911-511-906","Alternate-SMS-number":"","fax-number":"03-5520331"},{"unit":"苗栗縣政府消防局","SMS-number":"0911-511-907","Alternate-SMS-number":"","fax-number":"037-271982"},{"unit":"臺中市政府消防局","SMS-number":"0911-511-908","Alternate-SMS-number":"","fax-number":"04-23820675"},{"unit":"彰化縣消防局","SMS-number":"0911-511-910","Alternate-SMS-number":"","fax-number":"04-7513126"},{"unit":"南投縣政府消防局","SMS-number":"0911-511-911","Alternate-SMS-number":"","fax-number":"049-2238034"},{"unit":"雲林縣消防局","SMS-number":"0911-511-912","Alternate-SMS-number":"","fax-number":"05-5351735"},{"unit":"嘉義市政府消防局","SMS-number":"0911-511-913","Alternate-SMS-number":"","fax-number":"05-2716638"},{"unit":"嘉義縣消防局","SMS-number":"0911-511-914","Alternate-SMS-number":"","fax-number":"05-3620359"},{"unit":"臺南市政府消防局","SMS-number":"0911-511-915","Alternate-SMS-number":"","fax-number":"06-2952154"},{"unit":"高雄市政府消防局","SMS-number":"0911-511-917","Alternate-SMS-number":"","fax-number":"07-8225911"},{"unit":"屏東縣政府消防局","SMS-number":"0911-511-919","Alternate-SMS-number":"","fax-number":"08-7655420"},{"unit":"宜蘭縣政府消防局","SMS-number":"0911-511-920","Alternate-SMS-number":"","fax-number":"03-9323175"},{"unit":"花蓮縣消防局","SMS-number":"0911-511-921","Alternate-SMS-number":"","fax-number":"03-8574962"},{"unit":"臺東縣消防局","SMS-number":"0911-511-922","Alternate-SMS-number":"","fax-number":"089-326923"},{"unit":"澎湖縣政府消防局","SMS-number":"0911-511-923","Alternate-SMS-number":"","fax-number":"06-9272457"},{"unit":"金門縣消防局","SMS-number":"0911-511-924","Alternate-SMS-number":"","fax-number":"082-371430"},{"unit":"連江縣消防局","SMS-number":"0919-919-995","Alternate-SMS-number":"","fax-number":"083-623816"}]}};

var coords = false;
var map = false;
var regions = false;
var helpmsg = false;
var sms = false;

function log(...msg) {
    console.log(...msg);
    //document.getElementById('console').innerHTML += msg.join(' ') + '<br/>';
}

function got_position(position) {
    coords = position.coords;
    document.getElementById('position').innerHTML = `緯度: ${coords.latitude}<br/>經度: ${coords.longitude}`;
    for (let p in coords) {
        log(`${p}: ${coords[p]}`);
    }
    /*
    let map = document.getElementById('map');
    let gmap_uri = "http://maps.googleapis.com/maps/api/staticmap?center=" +
        coords.latitude + "," +
        coords.longitude + "&zoom=13&size=300x300&sensor=false";
    log('google map:', gmap_uri);
    map.href = gmap_uri;
    map.innerHTML = '查看地圖';
    let img = new Image();
    img.src = gmap_uri;
    map.appendChild(img); // This service requires an API key.
    */

    document.getElementById('embedMap').src = `http://www.google.com.tw/maps?q=loc:${coords.latitude},${coords.longitude}&output=embed`;

    set_sms_number();
}

/*
+ RFC5724: SMS https://tools.ietf.org/html/rfc5724
+ RFC3966: telphone number
+ click to call: https://developers.google.com/web/fundamentals/native-hardware/click-to-call/
 */
function set_sms_number() {
    let regions = document.getElementById('regions');
    let position = (coords ? `GPS:${coords.latitude},${coords.longitude}` : '');
    let sms = regions.options[regions.selectedIndex].value;
    let msg = document.getElementById('helpmsg').value.trim();
    let sms_href = `sms:${sms}?body=` +
        encodeURIComponent(`求救！ ${position} ${msg}`);
    log(sms_href);
    document.getElementById('sms').href = sms_href;

    if (localStorage) {
        log(`store ${sms}`);
        localStorage.setItem('sms', sms);
    }
}

function helpmsg_changed() {
    if (localStorage) {
        log("store helpmsg");
        localStorage.setItem('helpmsg', document.getElementById('helpmsg').value.trim());
    }

    set_sms_number();
}

function create_regions() {
    // TODO save list in localStorage.
    let regions = document.getElementById('regions');
    let option;
    let default_sms = false;
    if (localStorage) {
        default_sms = localStorage.getItem('sms');
        //log('default sms ', default_sms);
    }

    regions.innerHTML = '';
    u119_list.result.records.forEach((v,i)=>{
        if (/^\D/.test(v['SMS-number']))
            return;
        //log(v.unit, v['SMS-number']);
        option = document.createElement('option');
        option.value = v['SMS-number'];
        option.textContent = v.unit;
        if (default_sms && default_sms == v['SMS-number']) {
            default_sms = v['SMS-number']
            option.selected = true;
        }
        regions.appendChild(option);
    });

    regions.addEventListener('change', set_sms_number);
}

function init() {
    log('init...');
    create_regions();

    if (localStorage) {
        let msg = localStorage.getItem('helpmsg');
        if (msg && msg.length > 0)
            document.getElementById('helpmsg').value = msg;
    }

    set_sms_number();

    // try to update list from internet.
    var xhr = new XMLHttpRequest();
    if (xhr.overrideMimeType)
        xhr.overrideMimeType('application/json');
    log(u119_api);
    xhr.addEventListener('load', function(){
        if (this.status >= 300)
            return;
        //log(this.responseText);
        let fd_list = JSON.parse(this.responseText);
        if (fd_list.success) {
            u119_list = fd_list;
            create_regions();
        }
    });
    xhr.addEventListener('error', function(evt){
        log(evt);
    });
    xhr.open('GET', u119_api, true);
    xhr.send();

    navigator.geolocation.getCurrentPosition(
        got_position,
        ()=>{},
        {
            enableHighAccuracy: true,
            timeout: 5000, // 5 seconds
            maximumAge: 0
        }
    );

    document.getElementsByClassName('row-sms')[0].addEventListener('click', ()=>{
        document.getElementById('sms').click();
    });

    document.getElementById('helpmsg').addEventListener('change', helpmsg_changed);
}

document.addEventListener('DOMContentLoaded', init, false);

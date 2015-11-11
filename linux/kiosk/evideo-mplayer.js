/*
evideo-mplayer.js Copyright (C) 2014 遊手好閒的石頭成 <shirock.tw@gmail.com>

evideo-mplayer.js is free software: you can redistribute it and/or modify
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
information about evideo-mplayer.js.
*/
"use strict";

/**
EVideo.playerPath: 
A field to direct the root URL/path of mplayer*.php.
Example. If you put mplayer*.php in 'http://localhost:1080/kiosk/player/', 
you should add the following line after evideo.js loaded.
EVideo.playerPath = 'http://localhost:1080/kiosk/player';

EVideo construct parameters:
* evideo - The evideo DOM node.
* src (optional) - media src. If not given, it will parse from evideo.

Attrib:
* src - path of video.
* poster - image of evideo.
  avconv -i video.mp4 -r 1 -ss 0 -f image2 snapshot.jpg
* keepaspect - keep aspect ratio. default is not.

Method:
* play()
* stop()

Event:
* play
* ended
* error

All event will get a parameter with the following fields:
* detail.src
* detail.mediaId
* detail.duration

 */
function EVideo(evideo, src)
{
    var self = this;
    this.node = evideo;

    if (!this.node.dataset)
        this.node.dataset = {};

    // Add control method to node.
    this.node.play = function() {
        //console.info('node.play ', this.dataset.src);
        EVideo._play(this);
    }
    
    // Add control method to node.
    this.node.stop = function() {
        //console.info('node.stop ', this.dataset.mediaId);
        if (this.dataset.mediaId)
            EVideo._stop(this.dataset.mediaId);
    }

    this.node.poster = function(poster_src) {
        EVideo._poster(this, poster_src);
    }

    if (!src) {
        var sources = this.node.getElementsByTagName('source');
        if (sources.length > 0) { // Firefox/IE/WebKit
            src = sources[0].getAttribute('src');
        }
        else { // Chrome does not allow to put <source> inside <evideo>.
            // Or if you don't put <source> inside <evideo>.
            src = this.node.getAttribute('src');
        }
    }
    //src = window.encodeURI(src);
    this.node.dataset.src = src;
    

    if (this.node.dataset.mediaId) // this node may be playing, stop it.
        EVideo._stop(this.node.dataset.mediaId);
    this.node.dataset.mediaId = '';
    /*
    放入 dataset 的資料，取出時的型態會變成字串。
    若 set mediaId = undefined, 則 get mediaId 的結果為 'undefined' ，在邏輯
    判斷時，反而為 true 。容易搞錯。
    但若放入 '' ，則取出後的邏輯判斷結為仍為 false ，結果明確。
    JS: 空字串視為 false.
    */

    /*
    if (this.node.dataset.timeoutId) // clear previous timeout.
        window.clearTimeout(this.node.dataset.timeoutId);
    this.node.dataset.timeoutId = '';
    */

    this.node.dataset.duration = '';

    this.width = this.node.getAttribute('width');
    this.height = this.node.getAttribute('height');

    this.node.style.display = 'block';
    this.node.style.width = this.width;
    this.node.style.height = this.height;

    this.poster();
}

/**
Static field of EVideo class.
It directs the root URL/path of mplayer*.php.
 */
EVideo.playerPath = '';

/**
Static method of EVideo class.
 */
EVideo._play = function(node) {
    var xhr = new XMLHttpRequest();
    if (xhr.overrideMimeType)
        xhr.overrideMimeType('text/plain');

    function fireEvent(ev) {
        var event = new CustomEvent(
            ev, 
            {
                bubbles: false,
                cancelable: false,
                detail: { // must name 'detail'
                    'src': node.dataset.src,
                    'mediaId': node.dataset.mediaId,
                    'duration': node.dataset.duration
                }
            }
        );
        console.debug(ev, node.dataset.src, node.dataset.mediaId);
        node.dispatchEvent(event);
    }

    xhr.onreadystatechange = function() {
        //console.info(this.status);
        if (this.status >= 300) {
            if (this.readyState == 4) {
                console.error("[%s] Could not load file %s", Date(), node.dataset.src);
                //node.innerHTML = "Could not load file " + node.dataset.src;
                fireEvent('error');
                //window.alert('error' + Date());
            }
            return false;
        }

        switch (this.readyState) {
        //case 0: // UNSENT
        //case 1: // OPENED
        /*case 3: // LOADING
            console.info('playing...');
            break;*/
        case 2: // HEADERS_RECEIVED
            node.dataset.mediaId = this.getResponseHeader('X-Media-Object-Id');
            node.dataset.duration = this.getResponseHeader('X-Media-Object-Duration');
            console.info(node.dataset.mediaId, node.dataset.duration);
            //node.innerHTML = '';
            /*node.dataset.timeoutId = window.setTimeout((function(id){
                return function(){
                    console.debug(id, node.dataset.mediaId);
                    if (id == node.dataset.mediaId) {
                        console.debug('time up', node.dataset.mediaId);
                        EVideo._stop(node.dataset.mediaId);
                        fireEvent('ended');
                    }
                };
            })(node.dataset.mediaId), node.dataset.duration + 5000);*/

            fireEvent('play');
            break;

        case 4: // DONE
            //console.info('xhr done', document.hidden, document.visibilityState);
            //console.info(this.responseText);
            //window.clearTimeout(node.dataset.timeoutId);
            //node.dataset.timeoutId = '';
            EVideo._stop(node.dataset.mediaId);
            node.dataset.mediaId = '';

            // if xmlhttprequest is aborted (or window unload), responstText will be null.
            if (this.responseText) {
                if (this.responseText.indexOf('Fatal error:') >= 0)
                    fireEvent('error');
                else
                    fireEvent('ended');
            }
            break;
        }
        return true;
    };

    var screen_top = 0, screen_left = 0;
    if (window.mozInnerScreenY) {
        screen_top = window.mozInnerScreenY;
        screen_left = window.mozInnerScreenX;
    }
    
    var mrl = [
        EVideo.playerPath + 'mplayer.php',
        '?src=', node.dataset.src,
        '&width=', node.clientWidth, 
        '&height=', node.clientHeight, 
        '&top=', node.offsetTop + node.clientTop + screen_top,
        '&left=', node.offsetLeft + node.clientLeft + screen_left,
        '&keep_aspect=', node.getAttribute('keepaspect')
    ].join('');
    console.debug(mrl);

    xhr.open('GET', mrl, true); // async call
    try {
        xhr.send();
    }
    catch (e) {
        console.error(e);
    }
}

/**
Static method of EVideo class.
 */
EVideo._stop = function(mediaId) {
    if (!mediaId || mediaId == 'undefined')
        return;
    var xhr = new XMLHttpRequest();
    var url = [
        EVideo.playerPath + 'mplayer_close.php',
        '?id=', mediaId
    ].join('');
    console.debug(url);
    xhr.open('GET', url, true); // async call
    try {
        xhr.send();
    }
    catch (e) {
    }
}

EVideo._poster = function(node, poster_src)
{
    if (!poster_src) {
        poster_src = node.getAttribute('poster');
        if (!poster_src) {
            var s = node.dataset.src;
            poster_src = s.substr(0, s.lastIndexOf('.')).concat('.jpg');
        }
    }
    console.debug("poster", poster_src);
    var style = node.style;
    style.backgroundSize = '100% 100%';
    style.backgroundImage = "url('"+poster_src+"')";
}

/**
EVideo class event listener. A wrapper of real evideo node.
 */
EVideo.prototype.addEventListener = function(ev, handler, bubbles) {
    this.node.addEventListener(ev, handler, bubbles);
}

/**
EVideo class event dispatcher. A wrapper of real evideo node.
 */
EVideo.prototype.dispatchEvent = function(ev) {
    this.node.dispatchEvent(ev);
}

/**
Instance method.
 */
EVideo.prototype.play = function() {
    EVideo._play(this.node);
}

/**
Instance method.
 */
EVideo.prototype.stop = function() {
    EVideo._stop(this.node.dataset.mediaId);
}

/**
Instance method.
 */
EVideo.prototype.poster = function(poster_src) {
    EVideo._poster(this.node, poster_src);
}

/**
Init evideo tags after loaded.
 */
document.addEventListener('DOMContentLoaded', function(){
    var nodes = document.getElementsByTagName('evideo');
    var evideos = [];
    for (var i = 0; i < nodes.length; ++i) {
        evideos[i] = new EVideo(nodes[i]);

        evideos[i].play(); // always autoplay

        /*
        evideos[i].addEventListener('play', function(ev){
            console.debug('play %s, %s', ev.detail.src, ev.detail.mediaId);
        }, false);
        evideos[i].addEventListener('ended', function(ev){
            console.debug('ended %s, %s', ev.detail.src, ev.detail.mediaId);
        }, false);
        evideos[i].addEventListener('error', function(ev){
            console.error('error %s', ev.detail.src);
        }, false);
        */
    }
}, false);

/**
Close all playing media when window close.
 */
window.addEventListener('unload', function(){
    var xhr = new XMLHttpRequest();
    var url = EVideo.playerPath + 'mplayer_close.php?id=all';
    console.debug(url);
    xhr.open('GET', url, false); // sync call
    try {
        xhr.send();
    }
    catch (e) {
    }
}, false);


/*
evideo-html5.js Copyright (C) 2014 遊手好閒的石頭成 <shirock.tw@gmail.com>

evideo-html5.js is free software: you can redistribute it and/or modify
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
information about evideo-html5.js.
*/

/**
construct parameters:
* evideo - The evideo DOM node.
* src (optional) - media src. If not given, it will parse from evideo.

method:
* play()
* stop()

event:
* play
* ended
* error
 */
function EVideo(evideo, src)
{
    var self = this;
    this.node = evideo;

    this.width = this.node.getAttribute('width');
    this.height = this.node.getAttribute('height');

    this.node.style.display = 'block';
    this.node.style.width = this.width;
    this.node.style.height = this.height;

    this.video_node = document.createElement('video');

    if (!src) {
        var sources = this.node.getElementsByTagName('source');
        // Chrome: <source> may be not allowed inside <evideo> and ignored.
        src = (sources.length > 0
            ? sources[0].src.substring(document.baseURI.lastIndexOf('/')+1)
            : this.node.getAttribute('src'));
    }
    //src = window.encodeURI(src);
    //console.info(src);
    if (EVideo.Root)
        src = EVideo.Root + src;
    this.video_node.src = src;
    console.debug('play ', this.video_node.src);
    
    this.node.innerHTML = ''; // clear all children inside 'evideo'.

    /* NOT allowed in IE/Midori(WebKit).
    for (var p in ev)
        sv[p] = ev[p];
    */
    /* NOT working
    for (var attr in ['width', 'height', 'autoplay', 'controls', 'loop']) {
        sv[attr] = ev.getAttribute(attr);
    }
    */
    this.video_node.width = this.width;
    this.video_node.height = this.height;

/*
    function add_video_event_handler(obj, src_ev, target_ev) {
        var ev_handler_code = obj.node.getAttribute(src_ev);
        if (!ev_handler_code)
            return;
        obj.video_node.addEventListener(target_ev, function(){
            //console.info(this.parentNode.getAttribute('onended'));
            var func = Function(ev_handler_code);
            func.apply(this.parentNode);
        }, false);
    }
*/
    function add_video_event_handler(ev_handler_code, video_node, target_ev) {
        if (!ev_handler_code)
            return;
        video_node.addEventListener(target_ev, function(){
            //console.info(this.parentNode.getAttribute('onended'));
            var func = Function(ev_handler_code);
            func.apply(this.parentNode);
        }, false);
    }

    var ev_handler_code = '';

    ev_handler_code = this.node.getAttribute('onended');
    add_video_event_handler(ev_handler_code, this.video_node, 'ended');

    ev_handler_code = this.node.getAttribute('onplay');
    add_video_event_handler(ev_handler_code, this.video_node, 'play');

    ev_handler_code = this.node.getAttribute('onerror');
    add_video_event_handler(ev_handler_code, this.video_node, 'error');

    //this.video_node.autoplay = this.node.getAttribute('autoplay');
    //this.video_node.controls = this.node.getAttribute('controls');
    //this.video_node.loop = this.node.getAttribute('loop');
    this.node.appendChild(this.video_node);

    //var autoplay = this.node.getAttribute('autoplay');
    //if (autoplay == 'autoplay' || autoplay == 'true' || autoplay == '1')
    this.video_node.play(); // always autoplay.
}

/**
EVideo class event listener. A wrapper of real evideo node.
 */
EVideo.prototype.addEventListener = function(ev, handler, bubbles) {
    this.video_node.addEventListener(ev, handler, bubbles);
}

/**
Instance method.
 */
EVideo.prototype.play = function() {
    this.video_node.play();
}

/**
Instance method.
 */
EVideo.prototype.stop = function() {
    this.node.innerHTML = '';
}

/**
Init evideo tags after loaded.
 */
document.addEventListener('DOMContentLoaded', function(){
    var nodes = document.getElementsByTagName('evideo');
    var evideos = [];
    for (var i = 0; i < nodes.length; ++i) {
        evideos[i] = new EVideo(nodes[i]);

        /*
        evideos[i].addEventListener('play', function(ev){
            console.debug('play %s', ev.target.src);
        }, false);
        evideos[i].addEventListener('ended', function(ev){
            console.debug('ended %s', ev.target.src);
        }, false);
        evideos[i].addEventListener('error', function(ev){
            console.error('error %s', ev.target.src);
        }, false);
        */
    }
}, false);

EVideo.Root = 'http://localhost/rock/Video/';


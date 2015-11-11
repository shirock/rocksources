#!/usr/bin/gjs
/*
$ ./host-ime.js file:///home/rock/Public/rocksources/web_input_method/embed-webkit-demo/form2.html

 See JavaScript 與Desktop - Desktop and WebKit
 http://blog.roodo.com/rocksaying/archives/14456843.html
*/
// apt-get install git1.0-gtk-2.0 gir1.0-webkit-1.0, gir1.0-soup-2.4
var GLib = imports.gi.GLib;
var Gtk = imports.gi.Gtk;
var WebKit = imports.gi.WebKit;
const WebKitLoadStatus = {
    WEBKIT_LOAD_PROVISIONAL: 0,
    WEBKIT_LOAD_COMMITTED: 1,
    WEBKIT_LOAD_FINISHED: 2,
    WEBKIT_LOAD_FIRST_VISUALLY_NON_EMPTY_LAYOUT: 3,
    WEBKIT_LOAD_FAILED: 4
};
/* status order when load page:
PROVISIONAL -> COMMITTED 
-> DOMContentLoaded (body.onload)
-> FIRST_VISUALLY_NON_EMPTY_LAYOUT -> FINISHED.
*/

function _script(f) {
    var ctx = f.toString();
    return ctx.slice(ctx.indexOf('{') + 1, ctx.lastIndexOf('}'));
}

function main() {
if (ARGV.length < 1) {
  print("you must give an URL");
  return;
}

GLib.set_prgname('hello webkit');
Gtk.init(0, null);
var w = new Gtk.Window();
w.connect("destroy", Gtk.main_quit); 

var view = new WebKit.WebView();
view.set_size_request(480, 600);
view.get_settings().set_property('enable-file-access-from-file-uris', true);

view.connect("notify::load-status", function() {
    //if (view.loadStatus == WebKitLoadStatus.WEBKIT_LOAD_FINISHED) { 
    if (view.loadStatus == WebKitLoadStatus.WEBKIT_LOAD_COMMITTED) {
        var external_src;
        external_src = GLib.file_get_contents('../touch-ime.js');
        view.execute_script(external_src[1]);
        /*
         See 撰寫乾淨的 eval 程式碼的技巧
         http://blog.roodo.com/rocksaying/archives/18991164.html‎
        */
        view.execute_script(_script(function(){
            if (document.URL.indexOf('file://') == 0) 
                TouchInputMethod.set_resources_path('file:///home/rock/Public/rocksources/web_input_method/');
            else
                TouchInputMethod.set_resources_path('http://localhost/rock/rocksources/web_input_method/');
        }));
        external_src = GLib.file_get_contents('../on-screen-keyboard.js');
        view.execute_script(external_src[1]);
    }
});

// WebKit: for security reason, you should give absolute path.
//view.load_uri("file:///home/rock/workspace/content/index.html");
//view.load_uri("http://www.twitter.com/");
view.load_uri(ARGV[0]);

w.add(view);
w.set_position(1); //GTK_WIN_POS_CENTER
w.show_all();
Gtk.main(); // gtk loop, quit by main_quit().
print("Goodbye");
}
main();


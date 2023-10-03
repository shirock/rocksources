/*
這一串 import 敘述，也可以用 importmap 來做。
但 importmap 的實現時間更晚，Firefox 在2022年12月，Safari在2023年3月才加入。
我個人不會現在(2023年)用啦。
*/
import { abc } from './modules/abc-script-type-module.js';

console.log("main module loaded");
// main entry
abc();
xyz();

// window load 還是可以用，但也要注意 module 空間
window.addEventListener('load', _=>{
    console.log('window loaded');

    abc();
    xyz();
});

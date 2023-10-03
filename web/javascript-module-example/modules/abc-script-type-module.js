// abc-script-type-module.js
function abc() {
    console.log("say ABC");
}

// script type 為 module 的程式碼，才能用 export 敘述
export { abc };
console.log("abc module loaded");

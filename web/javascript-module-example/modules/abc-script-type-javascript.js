// abc-script-type-javascript.js
function abc() {
    console.log("say ABC");
}

// 因為載入的 script type 為 text/javascript ，故不能用 export 敘述
// export { abc };
console.log("abc module loaded");

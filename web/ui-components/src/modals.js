/*
<link rel="stylesheet" href="https://stackpath.bootstrapcdn.com/bootstrap/4.3.1/css/bootstrap.min.css" integrity="sha384-ggOyR0iXCbMQv3Xipma34MD+dH/1fQ784/j6cY/iJTQUOhcWr7x9JvoRxT2MZw1T" crossorigin="anonymous">
<script src="https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js"></script>
<script src="https://stackpath.bootstrapcdn.com/bootstrap/4.3.1/js/bootstrap.min.js" integrity="sha384-JjSmVgyd0p3pXB1rRibZUAYoIIy6OrQ6VrjIEaFf/nJGzIxFDsf4x0xIM+B07jRM" crossorigin="anonymous"></script>
*/
/**
Spinner 待處理中的轉圈動畫

方法
----

+ Spinner.show(msg) 在畫面中央顯示一個代表處理中的轉圈圖示。
  它不和使用者互動，使用者不能自己關掉。
  必須呼叫 Spinner.hide() 隱藏。
+ Spinner.hide()

自訂外觀
--------

Spinner 會自行建立一個預設的 modal 外觀。但使用者也可以自訂。

參考 https://getbootstrap.com/docs/4.0/components/modal/ 自訂 Spinner 外觀。

+ Spinner 要求一個 id 為 "rock-spinner-modal" 的 Bootstrap modal 。
+ 若 modal 中包含一個 id 為 "rock-spinner-modal-body" 的區塊，將用於顯示 show() 方法的 msg 內容。
+ 建議不要加上 fade 效果。
  如果呼叫 hide() 時還沒有跑完 fade 的視覺效果，你將會發現 hide() 沒有作用。
+ 建議不要加上關閉控制項(具 data-dismiss 屬性的元素)。
  以免使用者在你的工作跑完之前就關掉 Spinner 效果。

Required: Bootstrap 4.3 or later
 */
const Spinner = (function(){
  let self = false;

  function __init__() {
    if (!document.getElementById('rock-spinner-modal')) {
      let div = document.createElement('div');
      div.innerHTML = `
        <div class="modal" id="rock-spinner-modal" data-backdrop="static" tabindex="-1" role="dialog" aria-hidden="true">
          <div class="modal-dialog modal-dialog-centered" role="document">
            <div class="modal-content">
              <div class="modal-body text-center">
                <div class="spinner-border text-success m-5" role="status" style="width: 3rem; height: 3rem;">
                  <span class="sr-only">Loading...</span>
                </div>
                <div id="rock-spinner-modal-body">
                </div>
              </div>
            </div>
          </div>
        </div>
      `;
      document.body.appendChild(div);
    }

    if (!self) {
      self = $('#rock-spinner-modal');
    }
  }

  function show(msg) {
    __init__();
    let msg_elm = document.getElementById('rock-spinner-modal-body');
    if (msg_elm)
      msg_elm.innerHTML = (msg ? msg : '');
    self.modal({show:true, backdrop:'static'});
  }

  function hide() {
    __init__();
    self.modal('hide');
  }

  return {
    show: show,
    hide: hide
  };
})();
// end Spinner

/**
彈出訊息窗

在使用者關閉訊息提示窗之前，你調用的每個 show() 訊息內容將會累加在提示窗中，而不是取代。
避免跳出太多訊息提示窗。

方法
----

+ MessageModal.show(msg)
+ MessageModal.hide()
+ MessageModal.title(content)

自訂外觀
--------

MessageModal 會自行建立一個預設的 modal 外觀。但使用者也可以自訂。

參考 https://getbootstrap.com/docs/4.0/components/modal/ 自訂 MessageModal 外觀。

+ MessageModal 要求一個 id 為 "rock-message-modal" 的 Bootstrap modal 。
+ modal 中必須包含一個 id 為 "rock-message-modal-body" 的區塊，用於顯示 msg 內容。
+ modal 可包含一個 id 為 "rock-message-modal-title" 的區塊，顯示視窗標題。

Required: Bootstrap 4.1 or later
 */
const MessageModal = (function(){
  let self = false;
  let self_body = false;
  let hide_binding = false;

  function __init__() {
    // 如果使用者沒有定義 id 為 rock-message-modal 的訊息區塊，就自己建一個。
    if (!document.getElementById('rock-message-modal')) {
      // console.log('create modal div');
      let div = document.createElement('div');
      div.innerHTML = `
        <div class="modal fade" id="rock-message-modal" tabindex="-1" role="dialog" aria-labelledby="rock-message-modal-title" aria-hidden="true">
          <div class="modal-dialog modal-dialog-centered" role="document">
            <div class="modal-content">
              <div class="modal-header">
                <h5 class="modal-title" id="rock-message-modal-title"></h5>
                <button type="button" class="close" data-dismiss="modal" aria-label="Close">
                  <span aria-hidden="true">&times;</span>
                </button>
              </div>
              <div id="rock-message-modal-body" class="modal-body">
              </div>
            </div>
          </div>
        </div>
      `;
      document.body.appendChild(div);
    }

    if (!self) {
      self = $('#rock-message-modal');
      self_body = document.getElementById('rock-message-modal-body');
    }

    if (!hide_binding) {
      hide_binding = true;
      self.on('hidden.bs.modal', function() {
        // console.log('hide message modal');
        self_body.innerHTML = '';
        self.modal('hide');
      });
    }
  }

  /**
  在使用者關閉訊息提示窗之前，調用 show() 要顯示的訊息內容，將會累加在提示窗中，而不是取代。
  最新的在最上列。
  避免跳出太多訊息提示窗。
   */
  function show(msg) {
    __init__();
    let div = document.createElement('div');
    div.innerHTML = msg;
    self_body.insertBefore(div, self_body.firstChild);

    self.modal('show');
  }

  function hide() {
    __init__();
    self.modal('hide');
  }

  function title(content) {
    __init__();
    let title_elm = document.getElementById('rock-message-modal-title');
    if (title_elm)
      title_elm.innerHTML = content;
  }

  return {
    show: show,
    hide: hide,
    title: title
  };
})(); 
// end MessageModal
<?php
class Book {
    public $book;
    
    /**
    the content will be injected by CommonGateway.
    @resource
     */
    public $request;
    
    /**
    the content will be injected by CommonGateway.
    @resource request_document_type
     */
    public $response_type;

    function get($isbn) {
        //echo "ISBN $isbn\n";
        $this->book->isbn = $isbn;
        $this->book->title = 'Book title';
        $this->book->author = 'rock';
    }
    
    function put($isbn) {
    }
    
    function post() {
        return array('type' => $this->response_type, 'form' => $this->request);
    }
    
    function delete($isbn) {
    }
}
?>

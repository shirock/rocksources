<?php
class Editor {
    function get($id) {
        $result = array(
            'book' => array(
                'isbn' => $id,
                'title' => 'Title of <Book>',
                'author' => 'rock'
            )
        );
        return $result;
    }
}
?>

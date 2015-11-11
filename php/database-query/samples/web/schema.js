{
    "books": {
        "_primaryKey": "isbn",
        "isbn": {
            "type": "string",
            "pattern": "^\\d{10,13}$",
            "default": false
        },
        "title": {
            "type": "string",
            "pattern": "^.+$",
            "default": false
        },
        "author": {
            "type": "string",
            "pattern": "^.+$",
            "default": false
        }
    }
}


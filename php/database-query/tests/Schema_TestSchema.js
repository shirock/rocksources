{
    "Database_RowTest": {
        "_primaryKey": "id",
        "_unique": "name",
        "id": {
            "type": "int",
            "pattern": "ctype_digit",
            "default": true,
            "notice": false
        },
        "name": {
            "type": "string",
            "pattern": "\\w{1,40}",
            "default": false,
            "notice": false
        },
        "password": {
            "type": "string",
            "pattern": false,
            "encoder": "Php::md5",
            "decoder": "DatabaseRowTest::maskPassword",
            "default": false,
            "notice": false
        },
        "permission": {
            "type": "int",
            "pattern": "ctype_digit",
            "default": 0,
            "notice": false
        },
        "email": {
            "type": "string",
            "pattern": "pattern_email",
            "default": "",
            "notice": ""
        }
    }
}

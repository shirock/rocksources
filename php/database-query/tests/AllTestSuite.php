<?php
if (!defined('PHPUnit_MAIN_METHOD')) {
    define('PHPUnit_MAIN_METHOD', 'AllTestSuite::main');
}

require_once 'PHPUnit/Framework.php';
require_once 'PHPUnit/TextUI/TestRunner.php';

class AllTestSuite
{
    public static function suite()
    {
        $testCases = array(
            'SchemaTest.php',
            'SchemaNullObjectTest.php',
            'SchemaTableTest.php',
            'SchemaTableColumnTest.php',
            'DatabaseQueryTest.php',
            'DatabaseQueryWithoutSchemaTest.php',
            'DatabaseQuerySelectArgumentsTest.php',
            'DatabaseRowTest.php',
            'DatabaseCompanyTest.php'
        );

        $suite = new PHPUnit_Framework_TestSuite;

        foreach ($testCases as $testCase) {
            echo "Add test case from $testCase.\n";
            require_once $testCase;
            preg_match('/(\w+)\.php$/', $testCase, $matches);
            $testCaseClassName = $matches[1];
            $suite->addTestSuite($testCaseClassName);
        }

        return $suite;
    }

    public static function main()
    {
        PHPUnit_TextUI_TestRunner::run(self::suite());
    }
}
if (PHPUnit_MAIN_METHOD == 'AllTestSuite::main') {
    AllTestSuite::main();
}

?>

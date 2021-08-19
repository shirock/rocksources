<?php
/**
 * 單純按引數位置安插字串，不考慮格式化行為。
 * 比 sprintf() 容易閱讀。
 */
function str_interpolate(string $subject, ...$values)
{
    $pairs = [];
    foreach ($values as $i => $v) {
        $pairs['{' . $i . '}'] = $v;
    }
    return strtr($subject, $pairs);
}

// echo str_interpolate('abc {0} {1} dd {2}x{0}', 'xyz', '456', 789), "\n";
// echo sprintf('abc %s %s dd %sx%s', 'xyz', '456', 789, 'xyz'), "\n";
?>
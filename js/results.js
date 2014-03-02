var Result = Result || {}

Result.results = [];
Result.results = [ 
    [ 
        { name : "CS10", section : "001", day: 0, start : 9, end : 11 },
        { name : "CS12", section : "001", day: 3, start : 13, end : 15 },
    ],
    [ 
        { name : "CS10", section : "001", day: 0, start : 14, end : 15 },
        { name : "CS12", section : "001", day: 3, start : 17, end : 19 },
    ],
    [ 
        { name : "CS10", section : "001", day: 0, start : 8, end : 12 },
        { name : "CS12", section : "001", day: 3, start : 11, end : 12 },
    ],
];
Result.current = 0;

Result.init = function() {
    var $right = $("#right-side");
    var $rbutton = $(document.createElement("button"));
    $rbutton.attr("type", "button")
            .attr("class", "btn btn-primary")
            .attr("onclick", "++Result.current; Result.init(); Result.generate();")
            .html("Next");
    $right.empty().append($rbutton);

    var $left = $("#left-side");
    var $lbutton = $(document.createElement("button"));
    $lbutton.attr("type", "button")
            .attr("class", "btn btn-primary pull-right")
            .attr("onclick", "Result.current > 0 ? --Result.current : window.location.reload(true); Result.init(); Result.generate();")
            .html("Previous");
    $left.empty().append($lbutton);
}

Result.generate = function() {
    var $table = $("#week-table").empty();
    /*
    var data = [ 
        { name : "CS10", section : "001", day: 0, start : 8, end : 12 },
        { name : "CS10", section : "001", day: 3, start : 13, end : 15 },
    ];*/
    
    var data = Result.results[Result.current];

    var $row = $(document.createElement("tr"));
    $row.append("<td>Time</td>")
        .append("<td>Monday</td>")
        .append("<td>Tuesday</td>")
        .append("<td>Wednesday</td>")
        .append("<td>Thursday</td>")
        .append("<td>Friday</td>")
        .append("<td>Saturday</td>")
        .append("<td>Sunday</td>")
    $table.append($row);
   
    var grid = [];

    for (var j = 0; j < 14; ++j) {
        var $row = $(document.createElement("tr"));
        grid.push([]);
        var $time = $(document.createElement("td"));
        if (j + 7 > 12) time = (j  + 7) % 12;
        else time = j + 7;
        $time.html(time.toString() + ":00");
        $row.append($time);
        for (var i = 0; i < 7; ++i) {
            var $cell = $(document.createElement("td"));
            grid[j].push($cell);
            $row.append($cell);
        }
        $table.append($row);
    }

    for (var i = 0; i < data.length; ++i) {
        for (var j = 0; j < data[i].end - data[i].start; ++j) {
            var $cell = $(grid[data[i].start - 7 + j][data[i].day]);
            if (data[i].name != "") {
                $cell.html(data[i].name + ":" + data[i].section);
                $cell.attr("class", "success");
            }
        }
    }
}

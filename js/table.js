var Table = Table || {};

Table.cell_id = 1000;
Table.cells = [];
Table.selection = 0;

Table.changeSelection = function(mode) {
    Table.selection = mode;
}

Table.generate = function() {
    var $table = $("#week-table");

    $table.html("");
    var $row = $(document.createElement("tr"));
    $row.append("<td id='" + (Table.cell_id++).toString() + "'>Monday</td>")
        .append("<td id='" + (Table.cell_id++).toString() + "'>Tuesday</td>")
        .append("<td id='" + (Table.cell_id++).toString() + "'>Wednesday</td>")
        .append("<td id='" + (Table.cell_id++).toString() + "'>Thursday</td>")
        .append("<td id='" + (Table.cell_id++).toString() + "'>Friday</td>")
        .append("<td id='" + (Table.cell_id++).toString() + "'>Saturday</td>")
        .append("<td id='" + (Table.cell_id++).toString() + "'>Sunday</td>")
    $table.append($row);

    var hour = 7; //Hour of...
    for (var i = 0; i < 14; ++i) {
        var $row = $(document.createElement("tr"));
        $row.attr("id", i.toString());
        for (var j = 0; j < 7; ++j) {
            var $cell = $(document.createElement("td"));
            $cell.attr("id", Table.cell_id.toString())
                 .attr("onmousedown", "Table.highlight(this)");
            $cell.append(hour.toString() + ":00");
            Table.cells.push({ obj: $cell, day: j, hour: i })
            Table.cell_id++;
            $row.append($cell);
        }
        $table.append($row);

        ++hour;
        if (hour >= 13) hour = 1;
    }
    console.log("generating...");
}

Table.highlight = function (cell) {
    $cell = $(cell);
    console.log("highlighting..." + $(cell).attr("id").toString());
    
    if ($cell.hasClass("success")) {
        $cell.removeClass("success");
        return;
    }
    if ($cell.hasClass("danger")) {
        $cell.removeClass("danger");
        return;
    }

    if (Table.selection == 0) 
        $(cell).addClass("success");
    else if (Table.selection == 1)
        $(cell).addClass("danger");
    else;
}

Table.encoded = function() {
    var encodedTable = "";
    for (var i = 0; i < Table.cells.length; ++i) {
        if (Table.cells[i].obj.hasClass("success")) 
            encodedTable += "02";
        else if (Table.cells[i].obj.hasClass("danger"))
            encodedTable += "00";
        else
            encodedTable += "01";
    }
   
    console.log(encodedTable + ",0,6,7,19");
    return encodedTable + ",0,6,7,19";
}

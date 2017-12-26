'use strict';

var createElement = function(type, id) {
  var out = document.createElement( type );
  out.id = id;
  return out;
};

$(document).ready(function() {

  // Main page structure
  var page = createElement('div', 'page');
  $('#page').addClass = 'container';
  var editor1 = createElement('div', 'editor1');
  var editor2 = createElement('div', 'editor2');
  $(document.body).append(page);
  $('#page').append(editor1);
  $('#page').append(editor2);
  
  // Set editor
  editor1.innerHTML = "(define hello \"hello\")";
  editor2.innerHTML = "var hello = 'hello';";
  var aceEditor1 = ace.edit(editor1);
  var aceEditor2 = ace.edit(editor2);
  aceEditor1.setTheme('ace/theme/clouds');
  aceEditor1.getSession().setMode('ace/mode/scheme');
  aceEditor1.setOptions({
    fontSize: '12pt'
  });
  aceEditor1.getSession().setUseWrapMode(true);
  
  aceEditor2.setTheme('ace/theme/clouds');
  aceEditor2.getSession().setMode('ace/mode/javascript');
  aceEditor2.setOptions({
    fontSize: '12pt'
  });
  aceEditor2.getSession().setUseWrapMode(true);

  $(window).resize(function() {
    aceEditor1.resize();
    aceEditor2.resize();
  });
  
  
  var socket = io.connect('http://localhost:3100');
  
  socket.emit()
  
  socket.on('setScheme', function(data) {
      aceEditor1.setValue(data);
      aceEditor1.clearSelection();
    });
    
  socket.on('setJS', function(data) {
      aceEditor2.setValue(data);
      aceEditor2.clearSelection();
    });
    
  aceEditor1.getSession().on('change', function() {
      socket.emit('schemeToJS', aceEditor1.getValue());
    });


});

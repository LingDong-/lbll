const fs = require('fs');

let egfnames = fs.readdirSync("examples").filter(x=>x.endsWith('.lbl'));
let EXAMPLES = Object.fromEntries(egfnames.map(x=>[x,fs.readFileSync("examples/"+x).toString()]));

delete EXAMPLES["blink.lbl"]
delete EXAMPLES["pwm.lbl"]

let CFUNS = Array.from(fs.readFileSync("lbll.h").toString().matchAll(/lbll_reg_cfun\( *"(.+?)"/g)).map(x=>x[1]);

let rm = fs.readFileSync("README.md")

var unmark = (t)=>(t
  //escape tags
  .replace(/\\</g,"&lt")
  .replace(/\\\|/g,"¢")
  .replace(/`#`/g,"ø")
  //tables
  .replace(/[\n\r^]((((.*\|)+.+)[\n\r$])((\||)((:|)\-+(:|)\|(:|))+\-+(:|)(\||)[\n\r])(((.*\|)+.+)[\n\r$])+)/g,'<p><table>\n$1</table></p>\n')
  .replace(/(\||)((:|)\-+(:|)\|(:|))+\-+(:|)(\||)[\n\r](?=((.*[\n\r])*<\/table>))/g,'')
  .replace(/(((.*\|)+.+))[\n\r$](?=((.*[\n\r])*<\/table>))/g,'  <tr>|$1|</tr>\n')
  .replace(/<tr>\|+(.*)\|+<\/tr>/g,'<tr> <td>$1</td> </tr>')
  .replace(/\|(?=((.+)<\/tr>))/g,'</td> <td>')
  //paragraph
  .replace(/([\n\r^](([^ !\+\*\-\=\#(\n)(\r)>(0-9)`(<.*>)]+.*[\n\r^])+))(?=[\n\r$])/g,'\n<p>$2</p>\n')
  //block quote
  .replace(/[\n\r^]> {0,1}> *(.*)/g,'\n> <blockquote>$1</blockquote>')
  .replace(/<\/blockquote>\n*> *<blockquote> *<\/blockquote>\n*> *<blockquote>/g,'<br>')
  .replace(/<\/blockquote>\n*> *<blockquote>/g,' ')
  .replace(/[\n\r^]> *(.*)/g,'\n<blockquote>$1</blockquote>')
  .replace(/<\/blockquote>\n*<blockquote> *<\/blockquote>\n*<blockquote>/g,'<br>')
  .replace(/<\/blockquote>\n*<blockquote>/g,' ')
  //fence code
  .replace(/[\n\r^]```(.*)[\n\r]((.*[\n\r])*?)```/g,'\n<pre lang="$1">$2</pre>')
  //setext header
  .replace(/(.+)[\n\r]=+[\n\r$]/g,'<h1>$1</h1>\n')
  .replace(/(.+)[\n\r]-+[\n\r$]/g,'<h2>$1</h2>\n')
  //atx header
  .replace(/###### *(.+)[\n\r$]/g,'<h6>$1</h6>\n')
  .replace(/##### *(.+)[\n\r$]/g,'<h5>$1</h5>\n')
  .replace(/#### *(.+)[\n\r$]/g,'<h4>$1</h4>\n')
  .replace(/### *(.+)[\n\r$]/g,'<h3>$1</h3>\n')
  .replace(/## *(.+)[\n\r$]/g,'<h2>$1</h2>\n')
  .replace(/# *(.+)[\n\r$]/g,'<h1>$1</h1>\n')
  //horizontal rule
  .replace(/[\n\r^]([\*\-\_] *){3,}[\n\r$]/g,'\n<hr></hr>\n')
  //unordered list
  .replace(/[\n\r^](((( ){4,}[\*\+\-] .+[\n\r$])((( ){4,}[\*\+\-] .+[\n\r$])|(( ){6,}.*[\n\r$])|[\n\r$])*))/g,'\n    <ul>\n$1    </ul>\n')
  .replace(/[\n\r^]( ){4,}([\*\+\-]) (.+)/g,'\n      <li>$3</li>')
  .replace(/[\n\r^](((( ){2,}[\*\+\-] .+[\n\r$])((( ){2,}[\*\+\-] .+[\n\r$])|(( ){4,}.*[\n\r$])|[\n\r$])*))/g,'\n  <ul>\n$1  </ul>\n')
  .replace(/[\n\r^]( ){2,}([\*\+\-]) (.+)/g,'\n    <li>$3</li>')
  .replace(/[\n\r^](((( ){0,}[\*\+\-] .+[\n\r$])((( ){0,}[\*\+\-] .+[\n\r$])|(( ){2,}.*[\n\r$])|[\n\r$])*))/g,'\n<ul>\n$1</ul>\n')
  .replace(/[\n\r^]( ){0,}([\*\+\-]) (.+)/g,'\n  <li>$3</li>')
  //ordered list
  .replace(/[\n\r^](((( ){4,}[0-9]+\. .+[\n\r$])((( ){4,}[0-9]+\. .+[\n\r$])|(( ){6,}.*[\n\r$])|[\n\r$])*))/g,'\n    <ol>\n$1    </ol>\n')
  .replace(/[\n\r^]( ){4,}([0-9]+\.) (.+)/g,'\n      <li>$3</li>')
  .replace(/[\n\r^](((( ){2,}[0-9]+\. .+[\n\r$])((( ){2,}[0-9]+\. .+[\n\r$])|(( ){4,}.*[\n\r$])|[\n\r$])*))/g,'\n  <ol>\n$1  </ol>\n')
  .replace(/[\n\r^]( ){3,}([0-9]+\.) (.+)/g,'\n    <li>$3</li>')
  .replace(/[\n\r^](((( ){0,}[0-9]+\. .+[\n\r$])((( ){0,}[0-9]+\. .+[\n\r$])|(( ){2,}.*[\n\r$])|[\n\r$])*))/g,'\n<ol>\n$1</ol>\n')
  .replace(/[\n\r^]( ){0,}([0-9]+\.) (.+)/g,'\n  <li>$3</li>')
  //em & strong & code
  .replace(/([^\\])__(.*?[^\n\r\\])__/g,'$1<strong>$2</strong>')
  .replace(/([^\\])\*\*(.*?[^\n\r\\])\*\*/g,'$1<strong>$2</strong>')
  .replace(/([^\\])\*(.*?[^\n\r\\])\*/g,'$1<em>$2</em>')
  .replace(/([^\\])`(.*?[^\n\r\\])`/g,'$1<code>$2</code>')
  //image & link
  .replace(/!\[\]\((.*?)\)/g,'<img src="$1" alt=""/>')
  .replace(/!\[(.*?)\]\((.*?)\)/g,'<figure><img src="$2" alt="$1"/><figcaption>$1</figcaption></figure>')
  .replace(/\[(.*?)\]\((.*?)\)/g,'<a href="$2">$1</a>')
  //escape
  .replace(/\\(\*|_|`)/g,'$1')
  .replace(/¢/g,"|")
  .replace(/ø/g,"<code>#</code>")
)

let smd = unmark(rm.toString().split("<!--BEGINSYNTAX-->")[1].split("<!--ENDSYNTAX-->")[0]);
let omd = unmark(rm.toString().split("<!--BEGINOP-->")[1].split("<!--ENDOP-->")[0]);


function main(){

  let eg0 = "line.lbl"
  let tobar = document.createElement("div");
  document.getElementById('td0').appendChild(tobar);
  tobar.style=`
  height:30px;
  border-bottom:1px solid black;
  display:table-cell; 
  vertical-align: middle;
  `

  let sel_f = document.createElement("select");
  for (let k in EXAMPLES){
    let opt = document.createElement("option");
    opt.innerHTML = k;
    sel_f.appendChild(opt);
  }
  sel_f.onchange = function(){
    src.value = EXAMPLES[sel_f.value];
    highlight();
  }
  sel_f.style=`
  margin-right:5px;
  `;
  sel_f.value = eg0;
  tobar.appendChild(sel_f);

  let btn_r = document.createElement("button");
  btn_r.innerHTML = "run";
  tobar.appendChild(btn_r);
  btn_r.style=`
  margin-right:5px;
  `
  btn_r.onclick = function(){
    ctx.clearRect(0,0,cnv.width,cnv.height);
    out.value="";
    log.value="";
    run(src.value);
  }
  
  let src = document.createElement("textarea");
  src.id = 'src';
  document.getElementById('td0').appendChild(src);
  
  
  src.style=`
  width:calc(100% - 8px);
  height:calc(100% - 40px);
  resize:none;
  border:none;
  font-size:13px;
  margin:4px;
  padding:0px;
  line-height:13px;
  color:rgba(0,0,0,0.05);
  caret-color: black;
  outline: none;
  `
  src.setAttribute("spellcheck","false");


  let out = document.createElement("textarea");
  out.id = "output";
  document.getElementById('td3').appendChild(out);
  out.style=`
  width:100%;
  height:100%;
  resize:none;
  border:none;
  white-space: pre-wrap;
  outline: none;
  `
  out.setAttribute("spellcheck","false");

  let log = document.createElement("textarea");
  log.id = "log";
  log.style=`
  width:100%;
  height:100%;
  resize:none;
  border:none;
  font-size:11px;
  outline: none;
  `
  log.setAttribute("spellcheck","false");

  document.getElementById('td1').appendChild(log);

  let cnv = document.createElement("canvas");
  cnv.id = "display";
  cnv.width = 128;
  cnv.height = 64;
  let ctx = cnv.getContext('2d');
  document.getElementById('td2').appendChild(cnv);

  cnv.style=`
  border:0.5px solid black;
  transform:scale(200%);
  transform-origin: center center;
  -webkit-transform:scale(2,2);
  pointer-events:none;
  image-rendering: pixelated;
  image-rendering: -moz-crisp-edges;
  image-rendering: crisp-edges;
  `;

  LBLL().then(function(lbll){
    window.init = lbll.cwrap('init', 'number', [])
    window.run = lbll.cwrap('run', 'number', ['string'])
    init();
    btn_r.click();
  })


  let dst = document.createElement("pre");
  document.body.appendChild(dst);
  document.getElementById("td0").appendChild(dst);
  dst.style=`
  pointer-events:none;
  position:absolute;
  left:1px;
  top:34px;
  margin:4px;
  padding:0px;
  font-size:13px;
  overflow:hidden;
  white-space:pre-wrap;
  width:calc(100% - 10px);
  height:calc(100% - 42px);
  line-height:13px;
  `

  if (/^((?!chrome|android).)*safari/i.test(navigator.userAgent)){ 
    // stupid safari
  }else{
    console.log("thank you for using a good browser.")
    src.style.whiteSpace="nowrap";
    dst.style.whiteSpace="pre";
    log.style.whiteSpace="nowrap";
    out.style.whiteSpace="nowrap";
  }


  src.value = EXAMPLES[eg0];
  

  let colors = {
    "color:crimson;":["@","%"],
    "color:tomato;":["@@","%%"],
    "color:royalblue":["^","->","=>","~","#","?","*"],
    "color:teal":CFUNS,
  }
  let keywords = [];
  for (let k in colors){
    colors[k].map(x=>keywords.push([x,k]));
  }
  keywords.sort((a,b)=>(b[0].length-a[0].length));
  keywords = Object.fromEntries(keywords);
  console.log(keywords);

  function highlight(){


    let src = document.getElementById('src');
    var cc = src.value;
    // console.log(cc);
    var nc = ""
    for (var j = 0; j < cc.length; j++){
      var matched = false
      for (var k in keywords){
        matched = true;
        var buf = "";
        for (var l = 0; l < k.length; l++){
          buf += cc[j+l];
          if (cc[j+l] != k[l]){
            matched = false;
            break;
          }
        }
        if (matched){ 
          nc += `<span style="${keywords[k]}">`+buf+"</span>";
          j += k.length-1;
          break;
        }
      }
      
      if (cc[j] == ';'){
        let buf = cc[j++];
        while (j < cc.length && cc[j] != ';'){
          buf += cc[j++];
        }
        if (cc[j]) buf += cc[j];
        nc += `<span style="color:gray">${buf}</span>`
        matched = 1;
      }
      if (cc[j] == '"'){
        let buf = cc[j++];
        while (j < cc.length && (cc[j] != '"' || cc[j-1] == '\\')){
          buf += cc[j++];
        }
        if (cc[j]) buf += cc[j];
        nc += `<span style="color:darkgoldenrod">${buf}</span>`
        matched = 1;
      }
      if (!matched){
        nc += cc[j];
      }
    }
    dst.innerHTML = nc+"\n"; 
  }
  src.onscroll = function(){
    dst.scrollLeft = src.scrollLeft;   
    dst.scrollTop = src.scrollTop;  
  }

  src.oninput = function(){
    highlight();
  }
  highlight();

  // setInterval(highlight,200);
}




let html = `
<html>
<head>
<style>
body{
  margin:20px;
  font-family:sans-serif;
}
textarea{
  font-family:monospace;
  display:block;
  line-break: auto;
}
#ref table{
  font-size:12px;
  color:black;
  width:100%;
}
#ref td{
  min-width:100px;
  padding-right:10px;
  border-top:1px solid whitesmoke;
}
#ref tr:first-child{
  font-variant: small-caps;
  font-size:14px;
}
#ref code{
  font-weight:bold;
}
</style>
</head>
<body>
<div style="font-size:24px">LBLL</div>
<table style="margin:auto;width:100%;max-width:1200px;height:600px;max-height:100%">
<tr style="height:20px;font-size:14px;font-variant: small-caps;"><td>source</td> <td>bytecode</td> <td>output</td></tr>
<tr>
  <td id="td0" rowspan="2" style="position:relative;border:1px solid black;;max-width:33%;min-width:254px;height:100%;"></td>
  <td id="td1" rowspan="2" style="border:1px solid black;;max-width:33%;min-width:254px;height:100%;"></td>
  <td id="td2"             style="border:1px solid black;;max-width:33%;min-width:254px;height:130px;text-align:center;background:white;"></td>
</tr><tr><td id="td3" style="border:1px solid black;"></td></tr></table>
<br><br>
<div id="ref" style="margin:auto;width:100%;max-width:1200px;height:600px;max-height:100%;"><div style="font-size:14px;font-variant: small-caps;">cheatsheet</div>${smd}${omd}</div>
<br><br><br><br><br><br><br><br><br><br><br><br><br><br>
</body>
<script src="lbll.js"></script>
<script>console.log("compiled at: ${new Date().toString().replace(/"/g,"")}");</script>
<script>
var EXAMPLES=${JSON.stringify(EXAMPLES)};
var CFUNS=${JSON.stringify(CFUNS)};
${main.toString()}
main();
</script>
</html>
`;

fs.writeFileSync("site/index.html", html);

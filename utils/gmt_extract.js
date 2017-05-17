fs = require('fs');
data = fs.readFileSync(file);
json = JSNO.parse(data);
gmt = []; Object.keys(json).forEach(function(a){ if(json[a].gmt){ gmt.push(a + ' ' + json[a].gmt)};});
fs.writeFileSync('gmt.dat', gmt.join('\n'));
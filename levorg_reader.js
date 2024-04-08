/// SUBARU Levorg 2nd gen log リーダ ////////////////////////////////////////////////////////

LogReaderInfo.push({
	Caption:	"Subaru Levorg (*.txt)",
	Filter:		"*.txt;*.txt.gz",
	ReaderFunc:	"ReadLevorgLog",
	Priority:	0x564e3543,
});

function ReadLevorgLog( Files ){

	Log.Time	= [];
	Log.Speed	= [];
	Log.Tacho	= [];
	Log.Shift	= [];
	Log.Accel	= [];
	Log.Brake	= [];
	Log.Angle	= [];

	var	Cnt = 0;
	
	for( var i = 0; i < Files.length; ++i ){
		var file = new File();
		try{
			file.Open( Files[ i ], "zr" );
		}catch( e ){
			MessageBox( "ファイルが開けません: " + Files[ i ] );
			return 0;
		}
		
		while( 1 ){
			var Param = file.ReadLine().replace( /[\x0D\x0A]/g, '' ).split( /[,\t]/ );
			if( file.IsEOF()) break;
			
			Log.Time[Cnt] = StrToUTC( Param[0] );
			Log.Speed[Cnt] = Param[1];
			Log.Tacho[Cnt] = Param[2];
			Log.Shift[Cnt] = Param[3];
			Log.Accel[Cnt] = Param[4];
			Log.Brake[Cnt] = Param[5];
			Log.Angle[Cnt] = Param[6];

			++Cnt;
		}
		file.Close();
	}
}

function StrToUTC( str ){
	// 2024-03-30 07:43:50.899
	str.match( /(\d+)-(\d+)-(\d+) (\d+):(\d+):(\d+)(\.\d+)/ );
	return Date.UTC(
		RegExp.$1, RegExp.$2 - 1, RegExp.$3,
		RegExp.$4, RegExp.$5, RegExp.$6, RegExp.$7 * 1000
	) + ( new Date ).getTimezoneOffset() * 60000;
}

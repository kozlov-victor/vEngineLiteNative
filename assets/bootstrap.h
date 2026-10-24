
//language=javascript
const char* bootstrapJs = R"JS(

window = this;
var _requestAnimationFrameGlobalCallBack = ()=>{};
window._nextTick = ()=>{
    _requestAnimationFrameGlobalCallBack(Date.now());
    window._taskQueue.drain();
};


_globalGL.texImage2D = (...args)=>{
    if (args.length===9) {
        _globalGL._texImage2D_9(...args);
    }
    else if (args.length===6) {
        args[5] = args[5]._bitmap.$id;
        _external._texImage2D_6(...args);
    }
    else throw new Error('wrong arguments for texImage2D invocation');
}

alert = (msg)=>_external._alert(msg);

//const glNameByName = {};
//Object.keys(_globalGL).forEach(key=>{
//    glNameByName[_globalGL[key]] = key;
//});
//
//Object.keys(_globalGL).forEach(key=>{
//    const orig = _globalGL[key];
//    if (typeof _globalGL[key]!=='function') return;
//    if (key==='getError') return;
//    _globalGL[key] = (...args)=>{
//        const result = orig(...args);
//        if (_globalGL.getError()!==0) {
//            const argsDebug = [];
//            args.forEach(arg=>{
//                argsDebug.push(glNameByName[arg]||arg);
//            });
//            throw new Error(`error invocation ${key} with args ${argsDebug} (${args})`);
//        } else {
//            //console.log(key,args);
//        }
//        return result;
//    }
//});

(()=>{

    const _prepareLogArgs = (args)=>{
        const arr = [];
        for (let a of args) {
            try {
                if (a?.apply) arr.push(""+a);
                if (a?.substr) arr.push(a);
                else arr.push(JSON.stringify(a));
            }
            catch(e){
                arr.push(""+a);
            }
        }
        return arr;
    }

    console.log = (...args)=>console._log(..._prepareLogArgs(args));
    console.error = (...args)=>console._error(..._prepareLogArgs(args));
    console.warn = (...args)=>console._warn(..._prepareLogArgs(args));

    class TaskQueue {

        constructor(){
            this._tasks = [];
        }

        addNextTask(fn) {
            this._tasks.push(fn);
        }

        drain() {
            for (const t of this._tasks) {
                t();
            }
            this._tasks.length = 0;
        }

    }

    window._taskQueue = new TaskQueue();
    window._drainTaskQueue = ()=>window._taskQueue.drain();

    class CanvasStyle {

        constructor(){

        }

        set width(val){
            _external.setSurfaceWidth(parseInt(val));
        }

        get width(){
            return _external.getSurfaceWidth();
        }

        set height(val){
            _external.setSurfaceHeight(parseInt(val));
        }

        get height(){
            return _external.getSurfaceHeight();
        }
    }

    class Canvas {
        constructor(){
            this.style = new CanvasStyle();
            this.clientWidth = this.style.width;
            this.clientHeight = this.style.height;
            this.ontouchstart = ()=>{};
            this.ontouchmove = ()=>{};
            this.ontouchend = ()=>{};
        }
        getContext(val){
            if (val==='webgl') return _globalGL;
            else return null;
        }
        setAttribute(){
        }
        addEventListener(){
        }
        set width(val){
            //this.style.width = val;
        }

        get width(){
            return this.style.width;
        }

        set height(val){
            //this.style.height = val;
        }

        get height(){
            return this.style.height;
        }
    }

    class Image {

        constructor(){
            this._src = null;
            this._onload = null;
            this.onerror = null;
            this.width = 0;
            this.height = 0;
        }

        set src(val){
            this._src = val;
            _taskQueue.addNextTask(()=>{
                const {id,width,height} = _external._loadBitmap(val);
                if (id!==0) {
                    this._bitmap = {$id:id};
                    this.width = width;
                    this.height = height;
                    this._onload && this._onload();
                }
                else this.onerror && this.onerror();
            });
        }

        get src(){
            return this._src;
        }

        set onload(cb) {
            this._onload = cb;
        }

        get onload(){
            return this._onload;
        }

    }
    window.Image = Image;

    const globalCanvas = new Canvas();
    _globalGL.canvas = globalCanvas;

    const bodyStyle = {};
    let bValue = undefined;
    Object.defineProperty(bodyStyle, "backgroundColor", {
          get() {
                return bValue;
          },
          set(newValue) {
                _external._setBgColor(newValue);
                bValue = newValue;
          },
          enumerable: true,
          configurable: true,
    });

    class Document {
        constructor(){
            this.body = {
                appendChild:()=>{},
                addEventListener:()=>{},
                style: bodyStyle
            }
        }
        createElement(val){
            if (val==='canvas') return globalCanvas;
            else throw new Error(`can not create element ${val}`);
        }
        getElementById(val){
            return globalCanvas;
        }
        querySelector(){
            return globalCanvas;
        }
        addEventListener(){
        }
    }

    class XMLHttpRequest {

        constructor() {
            this.status = 0;
            this.url = '';
        }

        setRequestHeader(){

        }

        open(method,url){
            this.url = url;
        }

        send(){
            const currUrl = this.url.split('?')[0];
            const successCallback = (resp)=>{
                this.readyState = 4;
                this.status = 200;
                this.response = resp;
                if (resp && resp.toUpperCase) this.responseText = resp;
                if (resp && this.responseType==='json') this.response = JSON.parse(resp);
                this.onload && this.onload();
                this.onreadystatechange && this.onreadystatechange();
            };
            _taskQueue.addNextTask(()=>{ // todo
                const resp = (this.responseType==='blob' || this.responseType==='arraybuffer')?
                    _external._loadBinary(currUrl):
                    _external._loadString(currUrl);
                successCallback(resp);
            });
        }

        getResponseHeader(){

        }

    };

    window.XMLHttpRequest = XMLHttpRequest;
    window.document = new Document();
    window.navigator = {
        platform:'vEngine',
        userAgent:'vEngine'
    };
    window.innerWidth = innerWidth;
    window.innerHeight = innerHeight;
    window.self = window.top = window;
    window.performance = {
        now:()=>Date.now(),
    };
    window.setTimeout = ()=>{};
    window.setInterval = ()=>{};
    window.scrollTo = ()=>{};
    window.requestAnimationFrame = (fn)=>{
        _requestAnimationFrameGlobalCallBack = fn;
    };


    let events = [];
    window.addEventListener = (name,cb)=>{
        events.push({name:name,cb:cb});
    };
    window.removeEventListener = (name,cb)=>{
        events.splice(events.indexOf(it=>it.cb===cb),1);
    }
    window._triggerEvent = (name,arg)=>{
        events.forEach(it=>{
            if (it.name===name) it.cb(arg);
        });
    }

})();

console.log("bootstrap.js loaded!");

)JS";
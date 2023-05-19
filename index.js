const express = require('express');
const multer = require('multer');
const bodyParser = require('body-parser');
const fs = require('fs');
const {exec} = require('child_process');

const app = express();

const storage = multer.diskStorage({
  destination: 'uploads/',
  filename: function(req, file, cb) {
    cb(null, file.originalname);  // Retain the original filename
  }
});

const upload = multer({storage: storage});  // Use the custom storage object

const workList = require('./worklist');  // Import the work list

// Create a 2D list of boolean values to track dispatched and received work
let dispatchedWork = [];
let receivedWork = [];

// Function to initialize dispatchedWork with false values
function initializeWorkArray() {
  let arr = [];
  // Fill the 2D list with false values
  for (let i = 0; i < workList.length; i++) {
    arr.push([]);
    for (let j = 0; j < workList[i].length; j++) {
      arr[i].push(false);
    }
  }

  return arr;
}

// Load the contents of received_work.txt when the server starts
function loadWorks() {
  if (fs.existsSync('received_work.txt')) {
    const fileData = fs.readFileSync('received_work.txt', 'utf8');
    receivedWork = JSON.parse(fileData);
    dispatchedWork = receivedWork;
    if (!Array.isArray(dispatchedWork) ||
        dispatchedWork.length !== workList.length) {
      console.error(
          'Invalid format in received_work.txt. Initializing dispatchedWork and receivedWork.');
      dispatchedWork = initializeWorkArray();
      receivedWork = initializeWorkArray();
    }
  } else {
    console.log(
        'received_work.txt not found. Initializing dispatchedWork and receivedWork.');
    dispatchedWork = initializeWorkArray();
    receivedWork = initializeWorkArray();
  }
}

// Save the contents of sentWork to received_work.txt every 10 seconds
function saveReceivedWork() {
  fs.writeFileSync('received_work.txt', JSON.stringify(receivedWork));
}

// Load the sentWork data and start the interval to save it
loadWorks();
setInterval(saveReceivedWork, 5000);

function getLatestWork() {
  // Return the indices of the first 'false' in dispatchedWork
  for (let i = 0; i < dispatchedWork.length; i++) {
    for (let j = 0; j < dispatchedWork[i].length; j++) {
      if (!dispatchedWork[i][j]) {
        return [i, j];
      }
    }
  }
  // If all work has been dispatched, return the length of the work list
  return [dispatchedWork.length, 0];
}

// If the directory 'renders' does not exist, create it
if (!fs.existsSync('renders')) {
  fs.mkdirSync('renders');
}

app.use(bodyParser.urlencoded({extended: true}));  // Parse URL-encoded bodies
app.use(bodyParser.json());                        // Parse JSON bodies

function combine_results(sublistNumber) {
  // Code to combine the results of the completed work in the specified
  // sublist Use the sublistNumber parameter to determine which sublist is
  // finished Perform any necessary operations on the completed work within
  // the sublist For example, you can access completedWork[work] to retrieve
  // the completed work data

  // Code goes here
  console.log(`Combining results for sublist ${sublistNumber}`);

  // Call `combine_work.out` with its first argument being the sublist number
  exec(`./combine_work.out ${sublistNumber}`, (err, stdout, stderr) => {
    if (err) {
      console.log(err);
    } else {
      console.log(stdout);
    }
  });
}

app.get('/get_work', async (req, res) => {
  const workIndices = getLatestWork();
  const currentSublistIndex = workIndices[0];
  const currentWorkIndex = workIndices[1];

  if (currentSublistIndex === workList.length) {
    // All work is finished
    console.log('All work is finished');
    return res.status(404).json({work: 'done'});
  }

  const work = workList[currentSublistIndex][currentWorkIndex];

  dispatchedWork[currentSublistIndex][currentWorkIndex] = true;  // Mark the work as sent

  console.log('Sent work: ' + currentSublistIndex + ' ' + currentWorkIndex);
  return res.status(200).json({work: work, which_sublist: currentSublistIndex, which_work: currentWorkIndex});
});

app.post('/upload', upload.single('file'), async (req, res) => {
  const whichSublist = parseInt(req.body.which_sublist);
  const whichWork = parseInt(req.body.which_work);

  // Print a log message that the /upload endpoint was called
  console.log(
      '/upload called with sublist ' + whichSublist + ' and work ' + whichWork);

  if (isNaN(whichSublist) || isNaN(whichWork)) {
    return res.status(400).json({
      message: 'Invalid sublist or work index: you entered ' + whichSublist +
          ' ' + whichWork
    });
  }

  // File has been uploaded and stored in req.file
  if (!req.file) {
    return res.status(400).json({message: 'No file uploaded'});
  }

  receivedWork[whichSublist][whichWork] = true;  // Mark the work as sent

  // Check if the current sublist is finished
  let isSublistFinished = receivedWork[whichSublist].every(work => work);

  if (isSublistFinished) {
    combine_results(
        whichSublist);  // Call combine_results for the finished sublist
  }

  return res.status(200).json({message: 'Work and file uploaded successfully'});
});

app.listen(3000, () => {
  console.log('Server listening on port 3000');
});

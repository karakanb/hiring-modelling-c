# hiring-modelling-c
A hiring simulation with C, Linux threads and processes.

# Problem Description

In a company new employees are hired after a written exam and an interview. A “Registrar”, a “Written Exams Committee (WEC)” and an “Interviewer” handle the whole hiring process. There is only 1 Registrar and only 1 Interviewer in the office.

A “Registrar” takes an application form from each applicant and places it in the “Applications Box”. Assume that the whole application process takes 3 minutes. To simulate the time, you can use the “sleep” command and treat minutes as seconds (i.e. sleep for 3 seconds to simulate 3 minutes).

The written exam consists of questions from T different topics. Each member of the WEC is responsible for preparing one question for only one topic. Therefore, the WEC consists of T members. Each member of the WEC should read the topic assignments from an input file. After learning the topic, each WEC member should prepare a written exam question on the
assigned topic and write the question to a question sheet. When all questions are ready, each
member of the WEC should check all other questions prepared by the other members and approve
them. When all questions are approved by the WEC, the preparation of the written exam
is completed.
--- You are required to model the WEC as a process with the members of the WEC as
threads of this process.
--- The question sheet where the members write their questions should be modeled as an
array where each item on the array consists of a question prepared by the corresponding
WEC member (e.g. the question at the 5th location on the array is prepared and
written by WEC member no. 5) and a number for approvals.
--- Preparing a question takes n minutes (sleep for n seconds to simulate this) and n depends
on the topic. To simulate questions for different topics, a WEC member thread
should read the time it will take to prepare the question from an input file.
--- After all the questions are prepared, the WEC members check all the questions concurrently
(i.e. multiple WEC members can examine the same question at the same
time). Each WEC member checks the questions prepared by the other members and
approves them (i.e. increases the number of approvals). Checking a question takes 1
minute regardless of its topic (sleep for 1 second to simulate this). The last WEC
member approving a question (i.e., approvals=T-1) needs to print on the screen that
question is ready.
--- In the input file which contains the topic assignments, each line consists of three
fields, i.e. the WEC member number, the topic assigned to that member and n (the
time it takes to prepare a question on that topic).

When all the questions are ready (i.e. approvals are also completed), the Registrar makes the
written exam. The exam takes M minutes. M is fixed and is given to the program as a command
line parameter. At the end of the exam, the Registrar writes the scores down in the application
forms by generating random integers between 0 and 50. 

When the written exam is completed, the “Interviewer” calls in each applicant for an interview. The Interviewer takes the application forms from the Applications Box. The interview for each applicant lasts for I minutes. I is fixed and is given to the program as a command line parameter. The Interviewer scores each applicant after the interview by generating a random integer between 0 and 50 and writes the score down in the applicant's application form.
When all interviews are finished, the Registrar calculates each applicant's total score based on his written exam score and his interview score. Assume that the whole scoring process takes 3 minutes (sleep for 3 seconds to simulate this).

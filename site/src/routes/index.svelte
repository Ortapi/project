
<script lang="ts">
    let isOn = false;
    async function onClick(on_off: boolean) {
      isOn = on_off;
      await fetch("/api/toggle-led", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ is_on: isOn }),
      });
    };

    //let disabled = false;
    //let promise = Promise.resolve([]);
    async function getData() {
    let response = await fetch("/api/proj-data");
    let users = await response.json();
    return users;
  }
  const promise = getData();
  
  // function handleClick() {
  //   promise = getData();
  //   disabled = true;
  // }

    //   let time_str = "waiting for connection";
    //   let temper_str = "waiting for connection";
    //   import { onMount } from "svelte";
    //   onMount(() => {
    //   const socket = new WebSocket("ws://my-esp32.local/ws");
    //   socket.onopen = () => socket.send("web socket SUCK");
    //   socket.onmessage = (event) => {

    //   const data = JSON.parse(event.data);
    //   time_str = data.time_str;
    //   temper_str = data.temperature_str;
    //   };
    // });
  </script>

<div class="bg-indigo-300 object-cover flex-col p-8 text-center ">

    <h1 class="text-xl" >Or Tapiero Final Project</h1>

</div>


<div class="bg-indigo-300 object-cover flex-col p-8 text-center ">
  <h2>TIME:</h2>
  {#await promise}
    <p>Loading...</p>
  {:then users}
    <p>{users.time_str}</p>
  {:catch error}
    <p style="color: red">{error.message}</p>
  {/await}
  </div>

  <div class="bg-indigo-300 object-cover flex-col p-8 text-center ">
    <h2>Temperature:</h2>
    {#await promise}
      <p>Loading...</p>
    {:then users}
      <p>{users.temperature_str}</p>
    {:catch error}
      <p style="color: red">{error.message}</p>
    {/await}
    </div>

<!-- <div class="bg-indigo-300 object-cover flex-col p-8 text-center ">
    <h2>TIME:</h2>
    <h2 class="font-mono text-6xl">
      <span style="{`${time_str}`}">
        {`${time_str}`}
      </span>
    </h2> 
</div>

<div class="bg-indigo-300 object-cover flex-col p-8 text-center ">
  <h2>TEMPERATURE:</h2>
  <h2 class="font-mono text-6xl">
    <span style="{`${temper_str}`}">
      {`${temper_str}`}
    </span>
  </h2>
</div> -->

<div class="bg-indigo-300 object-cover flex-col p-8 text-center ">
    <h2>Switch On/Off:</h2>
    {#if isOn}
    <input on:click={() => onClick(false)} type="checkbox" class="toggle toggle-lg" checked>
    {:else}
    <input on:click={() => onClick(true)} type="checkbox" class="toggle toggle-lg" unchecked>
    {/if}

    <!-- <h2>Push to Refresh data:</h2>
    <button class="btn btn-active" on:click = {handleClick} {disabled}>refresh</button> -->
</div>






